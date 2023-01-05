#pragma once
#include <idt.h>

#define DEV_OFF_DATA 0
#define DEV_OFF_ERR 1
#define DEV_OFF_FEAT 1
#define DEV_OFF_SC 2
#define DEV_OFF_SN 3
#define DEV_OFF_CL 4
#define DEV_OFF_CH 5
#define DEV_OFF_DSEL 6
#define DEV_OFF_STAT 7
#define DEV_OFF_CMD 7

enum dev_type {
    DEV_PATA,
    DEV_SATA,
    DEV_PATAPI,
    DEV_SATAPI,
    DEV_UNKNOWN,
    DEV_DISCONNECTED,
    DEV_UNINTIALIZED
};

typedef struct device {
    uint16_t base;
    uint16_t dev_ctl;
    uint8_t master_slave_bit;
    uint64_t type;
    char* dev_name;
    device(uint16_t b, uint16_t ctl, uint8_t ms, char* name);
} device_t;

typedef struct partition {
    uint8_t attributes;
    uint8_t start_chs_high;
    uint8_t start_chs_mid;
    uint8_t start_chs_low;
    uint8_t type;
    uint8_t end_chs_high;
    uint8_t end_chs_mid;
    uint8_t end_chs_low;
    uint32_t lba;
    uint32_t sectors;
} partition_t;

typedef struct mbr {
    uint32_t uid;
    uint16_t zero;
    partition_t partition0;
    partition_t partition1;
    partition_t partition2;
    partition_t partition3;
    uint16_t signature;
} mbr_t;

void init_disk(device_t* dev);
void reset_device(device_t* dev);
void print_device(device_t* dev);
void read_disk(device_t* dev, uint8_t* buffer, uint32_t lba, uint8_t sectors);
void write_disk(device_t* dev, uint8_t* buffer, uint32_t lba, uint8_t sectors);
void print_mbr(mbr_t* mbr);