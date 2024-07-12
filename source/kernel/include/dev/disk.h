#ifndef DISK_H
#define DISK_H

#include "comm/types.h"
#include "cpu/irq.h"
#include "ipc/mutex.h"
#include "ipc/sem.h"

#define DISK_NAME_SIZE                  32
#define DISK_PRIMARY_PART_CNT           (4+1)
#define DISK_PART_NAME_SIZE             32
#define DISK_CNT                        2
#define MBR_PRIMARY_PART_NR             4

#define DISK_PER_CHANNEL                2
#define IOBASE_PRIMARY                  0x1F0

#define DISK_DATA_REG(disk)             (disk->port_base + 0)
#define DISK_ERR_REG(disk)              (disk->port_base + 1)
#define DISK_SECTOR_CNT_REG(disk)       (disk->port_base + 2)
#define DISK_LBA_LO(disk)               (disk->port_base + 3)
#define DISK_LBA_MID(disk)              (disk->port_base + 4)
#define DISK_LBA_HI(disk)               (disk->port_base + 5)
#define DISK_DRIVE(disk)                (disk->port_base + 6)
#define DISK_STATUS(disk)               (disk->port_base + 7)
#define DISK_CMD(disk)                  (disk->port_base + 7)

#define DISK_CMD_IDENTIFY               0xEC
#define DISK_CMD_READ                   0x24
#define DISK_CMD_WRITE                  0x34

#define DISK_STATUS_ERR                 (1 << 0)
#define DISK_STATUS_DRQ                 (1 << 3)
#define DISK_STATUS_DF                  (1 << 5)
#define DISK_STATUS_BUSY                (1 << 7)

#define DISK_DRIVE_BASE                 0xE0

#pragma pack(1)

typedef struct _part_item_t {
    uint8_t boot_active;
    uint8_t start_header;
    uint16_t start_sector : 6;
    uint16_t start_cylinder : 10;
    uint8_t system_id;
    uint8_t end_header;
    uint16_t end_sector : 6;
    uint16_t end_cylinder : 10;
    uint32_t relative_sectors;
    uint32_t total_sectors;
}part_item_t;


typedef struct _mbr_t {
    uint8_t code[446];
    part_item_t part_item[MBR_PRIMARY_PART_NR];
    uint8_t boot_sig[2];
}mbr_t;

#pragma pack()

struct _disk_t;

typedef struct _partinfo_t {
    char name[DISK_PART_NAME_SIZE];
    struct _disk_t * disk;
    int start_sector;
    int total_sector;
    enum {
        FS_INVALID = 0x00,
        FS_FAT16_0 = 0x06,
        FS_FAT16_1 = 0x0E,
    }type;
}partinfo_t;

typedef struct _disk_t {
    char name[DISK_NAME_SIZE];
    int sector_size;
    int sector_count;
    partinfo_t partinfo[DISK_PRIMARY_PART_CNT];
    enum {
        DISK_MASTER = (0 << 4),
        DISK_SLAVE = (1 << 4),
    }drive;
    uint16_t port_base;

    mutex_t * mutex;
    sem_t * op_sem;
}disk_t;


void disk_init(void);
void exception_handler_ide_primary(void);
void do_handler_ide_primary(exception_frame_t * frame);

#endif