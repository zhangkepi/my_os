#include "fs/fs.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "comm/boot_info.h"
#include "core/task.h"
#include "cpu/mmu.h"
#include "dev/console.h"
#include "dev/dev.h"
#include "fs/file.h"
#include "ipc/mutex.h"
#include "sys/_default_fcntl.h"
#include "sys/_intsup.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "tools/log.h"
#include <sys/file.h>

#define     TEMP_FILE_ID        100
#define     FS_TABLE_SIZE       10

static uint8_t TEMP_ADDR[100 * 1024];
static uint8_t * temp_pos;

extern fs_op_t devfs_op;
static list_t mounted_list;
static fs_t fs_table[FS_TABLE_SIZE];
static list_t free_list;


static fs_op_t * get_fs_op(fs_type_t type, int major) {
    switch (type) {
        case FS_DEVFS:
            return &devfs_op;
        default:
            break;
    }
    return (fs_op_t *)0;
}

static fs_t * mount(fs_type_t fs_type, char * mount_point, int major, int minor) {
    fs_t * fs = (fs_t *)0;

    log_printf("mount file system, name: %s, dev: 0x%x", mount_point, major);

    list_node_t * curr = list_first(&mounted_list);
    while (curr) {
        fs_t * fs = field_2_parent(curr, fs_t, node);
        if (kernel_memcmp(fs->mount_point, mount_point, FS_MOUNTP_SIZE) == 0) {
            log_printf("fs already mounted");
            goto mount_failed;
        }
        curr = list_node_next(curr);
    }

    list_node_t * free_node = list_remove_first(&free_list);
    if (!free_node) {
        log_printf("no free fs, mount failed.");
        goto mount_failed;
    }
    fs = field_2_parent(free_node, fs_t, node);

    fs_op_t * fs_op = get_fs_op(fs_type, major);
    if (fs_op == (fs_op_t *)0) {
        log_printf("unsupported fs type: %d", fs_type);
        goto mount_failed;
    }

    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_memcpy(mount_point, fs->mount_point, FS_MOUNTP_SIZE);
    fs->op = fs_op;

    if (fs->op->mount(fs, major, minor) < 0) {
        log_printf("mount fs %s failed.", mount_point);
        goto mount_failed;
    }
    list_insert_last(&mounted_list, &fs->node);

    return fs;
mount_failed:
    if (fs) {
        list_insert_last(&free_list, &fs->node);
    }
    return (fs_t *)0;
}

static void mounted_list_init(void) {
    list_init(&free_list);
    for (int i = 0; i < FS_TABLE_SIZE; i++) {
        list_insert_first(&free_list, &fs_table[i].node);
    }
    list_init(&mounted_list);
}

//从磁盘第sector扇区开始读取sector_count个扇区的数据
static void read_disk(uint32_t sector, int sector_count, uint8_t * buff) {
    // 配置要读取的磁盘，工作模式
    outb(0x1F6, 0xE0);

    // 从哪个扇区开始读，读多少个扇区
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    // 从哪个扇区开始读，读多少个扇区
    outb(0x1F2, (uint8_t)sector_count);
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    // 发送读取命令
    outb(0x1F7, 0x24);

    uint16_t * data_buff = (uint16_t *)buff;
    while(sector_count-- > 0) {
        // 磁盘未就绪一直等待
        while ((inb(0x1F7) & 0x88) != 0x8) {}

        // 一个扇区大小512字节，每次读两个字节
        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            *data_buff++ = inw(0x1F0);
        }
    }
}

static int is_path_valid(const char * path) {
    if ((path == (char *)0) || (path[0] == '\0')) {
        return 0;
    }
    return 1;
}

static void fs_protect(fs_t * fs) {
    if (fs->mutex) {
        mutex_lock(fs->mutex);
    }
}

static void fs_unprotect(fs_t * fs) {
    if (fs->mutex) {
        mutex_unlock(fs->mutex);
    }
}

static int is_fd_bad(int fd) {
    return fd < 0 || fd >= TASK_OFILE_NR;
}

static int path_begin_with(const char * path, const char * str) {
    const char * s1 = path, * s2 = str;
    while (*s1 && *s2 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s2 == '\0';
}

int path_2_num(const char * path, int * num) {
    int n = 0;
    const char * c = path;

    while (*c) {
        n = (n * 10 + *c) - '0';
        c++;
    }

    *num = n;

    return 0;
}

const char * path_next_child(const char * path) {
    const char * c = path;
    while (*c && (*c == '/')) {c++;}
    while (*c && (*c != '/')) {c++;}
    while (*c && (*c == '/')) {c++;}

    return *c ? c : (const char *)0;
}

int sys_open(const char * name, int flags, ...) {
    if (kernel_strncmp(name, "/shell.elf", 6) == 0) {
        read_disk(5000, 80, TEMP_ADDR);
        temp_pos = TEMP_ADDR;
        return TEMP_FILE_ID;
    }

    file_t * file = file_alloc();
    if (!file) {
        return -1;
    }
    int fd =  task_alloc_fd(file);
    if (fd < 0) {
        goto sys_open_failed;
    }
    fs_t * fs = (fs_t *)0;
    list_node_t * node = list_first(&mounted_list);
    while (node) {
        fs_t * curr = field_2_parent(node, fs_t, node);
        if (path_begin_with(name, curr->mount_point)) {
            fs = curr;
            break;
        }
        node = list_node_next(&curr->node);
    }

    if (fs) {
        name = path_next_child(name);
    } else {
    
    }

    file->mode = flags;
    file->fs = fs;
    char * n = (char *)name;
    kernel_strncpy(n, file->name, FILE_NAME_SIZE);

    fs_protect(fs);
    int err = fs->op->open(fs, name, file);
    if (err < 0) {
        fs_unprotect(fs);
        goto sys_open_failed;
    }
    fs_unprotect(fs);

    return fd;
sys_open_failed:
    file_free(file);
    if (fd >= 0) {
        task_remove_fd(fd);
    }
    return -1;
}


int sys_read(int file, char * ptr, int len) {
    if (file == TEMP_FILE_ID) {
        kernel_memcpy(temp_pos, ptr, len);
        temp_pos += len;
        return len;
    } 
    if (is_fd_bad(file) || !ptr || !len) {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }
    if (p_file->mode == O_WRONLY) {
        log_printf("file is write only.");
        return -1;
    }
    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int err = fs->op->read(ptr, len, p_file);
    fs_unprotect(fs);
    return err;
}


int sys_write(int file, char * ptr, int len) {
    if (is_fd_bad(file) || !ptr || !len) {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }
    if (p_file->mode == O_RDONLY) {
        log_printf("file is read only.");
        return -1;
    }
    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int err = fs->op->write(ptr, len, p_file);
    fs_unprotect(fs);
    return err;
}

int sys_lseek(int file, int ptr, int dir) {
    if (file == TEMP_FILE_ID) {
        temp_pos = TEMP_ADDR + ptr;
        return 0;
    }
    if (is_fd_bad(file)) {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }
    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int err = fs->op->seek(p_file, ptr, dir);
    fs_unprotect(fs);
    return err;
}


int sys_close(int file) {
    if (file == TEMP_FILE_ID) {
        return 0;
    } 
    if (is_fd_bad(file)) {
        log_printf("bad file %d", file);
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }
    ASSERT(p_file->ref > 0);
    if (p_file->ref-- == 1) {
        fs_t * fs = p_file->fs;
        fs_protect(fs);
        fs->op->close(p_file);
        fs_unprotect(fs);
        file_free(p_file);
    }
    task_remove_fd(file);
    return 0;
}

int sys_isatty(int file) {
    if (is_fd_bad(file)) {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }
    return p_file->type == FILE_TTY;
}

int sys_fstat(int file, struct stat * st) {
    if (is_fd_bad(file)) {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }

    kernel_memset(st, 0, sizeof(struct stat));
    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int err = fs->op->stat(p_file, st);
    fs_unprotect(fs);
    return err;
}

void fs_init(void) {
    mounted_list_init();
    file_table_init();

    fs_t * fs = mount(FS_DEVFS, "/dev", 0, 0);
    ASSERT(fs != (fs_t *)0);
}

int sys_dup(int file) {
    if (is_fd_bad(file)) {
        log_printf("file %d is not valid", file);
        return -1;
    }
    file_t * p_file = task_file(file);
    if (!p_file) {
        log_printf("file not opened");
        return -1;
    }
    int fd = task_alloc_fd(p_file);
    if (fd >= 0) {
        file_inc_ref(p_file);
        return fd;
    }
    log_printf("no task file available");
    return -1;
}