#include "dev/disk.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "core/task.h"
#include "cpu/irq.h"
#include "dev/dev.h"
#include "ipc/mutex.h"
#include "ipc/sem.h"
#include "sys/syslimits.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "comm/types.h"


static mutex_t mutex;
static sem_t op_sem;
static disk_t disk_buf[DISK_CNT];
static int task_on_open;


static void disk_send_cmd(disk_t * disk, uint32_t start_sector, uint32_t sector_cnt, int cmd) {
    outb(DISK_DRIVE(disk), DISK_DRIVE_BASE | disk->drive);

    outb(DISK_SECTOR_CNT_REG(disk), (uint8_t)(sector_cnt >> 8));
    outb(DISK_LBA_LO(disk), (uint8_t)(start_sector >> 24));
    outb(DISK_LBA_MID(disk), 0);
    outb(DISK_LBA_HI(disk), 0);

    outb(DISK_SECTOR_CNT_REG(disk), (uint8_t)(sector_cnt));
    outb(DISK_LBA_LO(disk), (uint8_t)(start_sector >> 0));
    outb(DISK_LBA_MID(disk), (uint8_t)(start_sector >> 8));
    outb(DISK_LBA_HI(disk), (uint8_t)(start_sector >> 16));

    outb(DISK_CMD(disk), (uint8_t)cmd);
}

static void disk_read_data(disk_t * disk, void * buf, int size) {
    uint16_t * c = (uint16_t *)buf;
    for (int i = 0; i < size / 2; i++) {
        *c++ = inw(DISK_DATA_REG(disk));
    }
}

static void disk_write_data(disk_t * disk, void * buf, int size) {
    uint16_t * c = (uint16_t *)buf;
    for (int i = 0; i < size / 2; i++) {
        outw(DISK_DATA_REG(disk), *c++);
    }
}

static int disk_wait_data(disk_t * disk) {
    uint8_t status;
    do {
        status = inb(DISK_STATUS(disk));
        if ((status & (DISK_STATUS_BUSY | DISK_STATUS_DRQ | DISK_STATUS_ERR)) != DISK_STATUS_BUSY) {
            break;
        }
    }while (1);
    return (status & DISK_STATUS_ERR) ? -1 : 0;
}

static int detect_part_info(disk_t * disk) {
    mbr_t mbr;

    disk_send_cmd(disk, 0, 1, DISK_CMD_READ);
    int err = disk_wait_data(disk);
    if (err < 0) {
        log_printf("read mbr failed.");
        return err;
    }
    disk_read_data(disk, &mbr, sizeof(mbr));
    part_item_t * item = mbr.part_item;
    partinfo_t * part_info = disk->partinfo + 1;
    for (int i = 0; i < MBR_PRIMARY_PART_NR; i++, item++, part_info++) {
        part_info->type = item->system_id;
        if (part_info->type == FS_INVALID) {
            part_info->start_sector = 0;
            part_info->total_sector = 0;
            part_info->disk = (disk_t *)0;
        } else {
            kernel_sprintf(part_info->name, "%s%d", disk->name, i + 1);
            part_info->start_sector = item->relative_sectors;
            part_info->total_sector = item->total_sectors;
            part_info->disk = disk;
        }
    }
    return 0;
}

static int identify_disk(disk_t * disk) {
    disk_send_cmd(disk, 0, 0, DISK_CMD_IDENTIFY);
    int err = inb(DISK_STATUS(disk));
    if (err == 0) {
        log_printf("%s does not exist", disk->name);
        return -1;
    }
    err = disk_wait_data(disk);
    if (err < 0) {
        log_printf("disk[%s] read failed", disk->name);
        return -1;
    }
    uint16_t buf[256];
    disk_read_data(disk, buf, sizeof(buf));
    disk->sector_count = *(uint32_t *)(buf + 100);
    disk->sector_size = SECTOR_SIZE;

    partinfo_t * part = disk->partinfo + 0;
    part->disk = disk;
    kernel_sprintf(part->name, "%s%d", disk->name, 0);
    part->start_sector = 0;
    part->total_sector = disk->sector_count;
    part->type = FS_INVALID;

    detect_part_info(disk);

    return 0;
}

static void print_disk_info(disk_t * disk) {
    log_printf("%s", disk->name);
    log_printf("  port base: 0x%x", disk->port_base);
    log_printf("  total size: %d m", disk->sector_count * disk->sector_size / 1024 / 1024);

    for (int i = 0; i < DISK_PRIMARY_PART_CNT; i++) {
        partinfo_t * part_info = disk->partinfo + i;
        if (part_info->type != FS_INVALID) {
            log_printf("    %s: type: 0x%x, start sector: %d, count: %d", part_info->name, part_info->type, part_info->start_sector, part_info->total_sector);
        }
    }
}

void disk_init(void) {
    log_printf("Check disk...");

    kernel_memset(disk_buf, 0, sizeof(disk_buf));

    mutex_init(&mutex);
    sem_init(&op_sem, 0);

    for (int i = 0; i < DISK_PER_CHANNEL; i++) {
        disk_t * disk = disk_buf + i;

        kernel_sprintf(disk->name, "sd%c", i + 'a');
        disk->drive = (i == 0) ? DISK_MASTER : DISK_SLAVE;
        disk->port_base = IOBASE_PRIMARY;

        disk->mutex = &mutex;
        disk->op_sem = &op_sem;

        int err = identify_disk(disk);
        if (err == 0) {
            print_disk_info(disk);
        }
    }
}


int disk_open(device_t * dev) {
    int disk_idx = (dev->minor >> 4) - 0xa;
    int part_idx = dev->minor & 0xF;

    if (disk_idx >= DISK_CNT || part_idx >= DISK_PRIMARY_PART_CNT) {
        log_printf("device minor error: %d", dev->minor);
        return -1;
    }

    disk_t * disk = disk_buf + disk_idx;
    if (disk->sector_count <= 0) {
        log_printf("disk not exist, device: sd%x", dev->minor);
        return -1;
    }

    partinfo_t * partinfo = disk->partinfo + part_idx;
    if (partinfo->total_sector <= 0) {
        log_printf("part not exist, device: sd%x", dev->minor);
        return -1;
    }

    dev->data = partinfo;

    irq_install(IRQ14_HARDDISK_PRIMARY, (irq_handler_t)exception_handler_ide_primary);
    irq_enable(IRQ14_HARDDISK_PRIMARY);

    return 0;
}

int disk_read(device_t * dev, int addr, char * buf, int size) {
    partinfo_t * partinfo = (partinfo_t *)dev->data;
    if (!partinfo) {
        log_printf("Get part info failed. device: %d", dev->minor);
        return -1;
    }
    disk_t * disk = partinfo->disk;
    if (disk == (disk_t *)0) {
        log_printf("No disk. device: %d", dev->minor);
        return -1;
    }

    mutex_lock(disk->mutex);

    task_on_open = 1;

    disk_send_cmd(disk, partinfo->start_sector + addr, size, DISK_CMD_READ);
    int cnt;
    for (cnt = 0; cnt < size; cnt++, buf += disk->sector_size) {
        if (task_current()) {
            sem_wait(disk->op_sem);
        }
        int err = disk_wait_data(disk);
        if (err < 0) {
            log_printf("disk(%s) read error: start sector %d, count: %d", disk->name, addr, size);
            break;
        }
        disk_read_data(disk, buf, disk->sector_size);
    }
    mutex_unlock(disk->mutex);
    return cnt;
}

int disk_write(device_t * dev, int addr, char * buf, int size) {
    partinfo_t * partinfo = (partinfo_t *)dev->data;
    if (!partinfo) {
        log_printf("Get part info failed. device: %d", dev->minor);
        return -1;
    }
    disk_t * disk = partinfo->disk;
    if (disk == (disk_t *)0) {
        log_printf("No disk. device: %d", dev->minor);
        return -1;
    }

    mutex_lock(disk->mutex);
    task_on_open = 1;
    disk_send_cmd(disk, partinfo->start_sector + addr, size, DISK_CMD_WRITE);
    int cnt;
    for (cnt = 0; cnt < size; cnt++, buf += disk->sector_size) {
        disk_write_data(disk, buf, disk->sector_size);
        if (task_current()) {
            sem_wait(disk->op_sem);
        }
        int err = disk_wait_data(disk);
        if (err < 0) {
            log_printf("disk(%s) read error: start sector %d, count: %d", disk->name, addr, size);
            break;
        }
    }
    mutex_unlock(disk->mutex);
    return cnt;
}

int disk_control(device_t * dev, int cmd, int arg0, int arg1) {
    return -1;
}
int disk_close(device_t * dev) {
    return 0;
}

void do_handler_ide_primary(exception_frame_t * frame) {
    pic_send_eoi(IRQ14_HARDDISK_PRIMARY);
    if (task_on_open && task_current()) {
        sem_notify(&op_sem);
    }
}


dev_desc_t dev_disk_desc = {
    .name = "disk",
    .major = DEV_DISK,
    .open = disk_open,
    .read = disk_read,
    .write = disk_write,
    .control = disk_control,
    .close = disk_close,
};
