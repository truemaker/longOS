#include <disk.h>
#include <io.h>
#include <vga.h>
#include <serial.h>
#include <timer.h>
#include <memory.h>
#include <heap.h>

device_t::device(uint16_t b, uint16_t ctl, uint8_t ms, char* name) {
    base = b;
    dev_ctl = ctl;
    master_slave_bit = ms;
    type = DEV_UNINTIALIZED;
    dev_name = name;
}

device_t::device() {
    base = 0;
    dev_ctl = 0;
    master_slave_bit = 0;
    type = DEV_UNINTIALIZED;
    dev_name = 0;
}

void select(device_t* dev) {
    outb(dev->base + DEV_OFF_DSEL, dev->master_slave_bit);
    for (uint64_t i = 0; i<15; i++) inb(dev->dev_ctl);
}

void select(device_t* dev, uint8_t flags) {
    outb(dev->base + DEV_OFF_DSEL, dev->master_slave_bit | flags);
    for (uint64_t i = 0; i<15; i++) inb(dev->dev_ctl);
}

dev_type determine_dev_type(device_t* dev,dev_type def) {
    reset_device(dev);
    select(dev);
    outb(dev->base+DEV_OFF_SC,0);
    outb(dev->base+DEV_OFF_SN,0);
    outb(dev->base+DEV_OFF_CL,0);
    outb(dev->base+DEV_OFF_CH,0);
    outb(dev->base+DEV_OFF_CMD,0xEC);
    if (inb(dev->dev_ctl) == 0) return DEV_DISCONNECTED;
    while (0x80 & inb(dev->dev_ctl));
    uint8_t cl = inb(dev->base + DEV_OFF_CL);
    uint8_t ch = inb(dev->base + DEV_OFF_CH);
    if (cl != 0 || ch != 0) {
        if (cl==0x14 && ch==0xEB) return DEV_PATAPI;
        if (cl==0x69 && ch==0x96) return DEV_SATAPI;
        return DEV_UNKNOWN_ATAPI;
    }
    while ((!(0x08 & inb(dev->dev_ctl))) && (0x01 & inb(dev->dev_ctl)));
    if (cl==0x14 && ch==0xEB) return DEV_PATAPI;
    if (cl==0x69 && ch==0x96) return DEV_SATAPI;
    if (cl==0 && ch==0) return DEV_PATA;
    if (cl==0x3c && ch==0xc3) return DEV_SATA;
    return def;
}

void init_disk(device_t* dev) {
    if (inb(dev->base + DEV_OFF_DATA) == 0xFF) {
        dev->type = determine_dev_type(dev,DEV_DISCONNECTED);
        return;
    }
    dev->type = determine_dev_type(dev,DEV_UNKNOWN);
}

void reset_device(device_t* dev) {
    print("[DISK] Resetting drive...\n\r");
    uint8_t dctl = inb(dev->dev_ctl);
    outb(dev->dev_ctl, dctl | 4);
    PIT::sleep(10);
    dctl &= ~4;
    outb(dev->dev_ctl, dctl);
    PIT::sleep(10);
}

void print_device(device_t* dev) {
    if (dev->type == DEV_DISCONNECTED) return;
    printf("Name: %s\n\rBase: %h\n\rControl: %h\n\rDrive: %c\n\rType: ",dev->dev_name,dev->base,dev->dev_ctl,(dev->master_slave_bit & 0x10) ? 'B' : 'A');
    switch(dev->type) {
        case DEV_DISCONNECTED: print("Not Connected"); break;
        case DEV_UNKNOWN: print("Unknown"); break;
        case DEV_PATA: print("PATA"); break;
        case DEV_PATAPI: print("PATA (Optical)"); break;
        case DEV_SATA: print("SATA"); break;
        case DEV_SATAPI: print("SATA (Optical)"); break;
        case DEV_UNINTIALIZED: print("Not initialized"); break;
    }
    print("\n\n\r");
}

void disk_delay(device_t* dev) {
    uint64_t k = 0;
    for (uint64_t i = 0; i < 16; i++) {
        k = inb(dev->dev_ctl);
    }
}

bool wait_disk_ready(device_t* dev) {
    uint8_t stat = inb(dev->dev_ctl);
    timer_t t;
    PIT::start_timer(&t);
    while (stat & (1 << 7) && !PIT::timer_expired(t,400)) stat = inb(dev->dev_ctl);
    if (inb(dev->dev_ctl) & 0x01) { print("[DISK] drive error!\n\r"); reset_device(dev); return true; }
    while (!(stat & (1 << 3)) && !PIT::timer_expired(t,400)) stat = inb(dev->dev_ctl);
    if (inb(dev->dev_ctl) & 0x01) { print("[DISK] drive error!\n\r"); reset_device(dev); return true; }
    if (PIT::timer_expired(t,400)) { reset_device(dev); return true; }
    return false;
}

void read_disk(device_t* dev, uint8_t* buffer, uint32_t lba, uint8_t sectors) {
    select(dev, 0xE0 | ((lba >> 24)&0x0F));
    outb(dev->base + DEV_OFF_SC, sectors);
    outb(dev->base + DEV_OFF_SN, lba);
    outb(dev->base + DEV_OFF_CL, lba >> 8);
    outb(dev->base + DEV_OFF_CH, lba >> 16);
    outb(dev->base + DEV_OFF_CMD, 0x20);
    if (wait_disk_ready(dev)) return read_disk(dev,buffer,lba,sectors);
    uint16_t* buffer16 = (uint16_t*)buffer;
    for (uint16_t i = 0; i < sectors; i++) {
        if (wait_disk_ready(dev)) return read_disk(dev,buffer,lba,sectors);
        for (uint16_t j = 0; j < 256; j++) {
            uint16_t word = inw(dev->base + DEV_OFF_DATA);
            *buffer16 = word;
            buffer16++;
        }
    }
}

void write_disk(device_t* dev, uint8_t* buffer, uint32_t lba, uint8_t sectors) {
    select(dev, 0xE0 | ((lba >> 24)&0x0F));
    outb(dev->base + DEV_OFF_FEAT, 0);
    outb(dev->base + DEV_OFF_SC, sectors);
    outb(dev->base + DEV_OFF_SN, lba);
    outb(dev->base + DEV_OFF_CL, lba >> 8);
    outb(dev->base + DEV_OFF_CH, lba >> 16);
    outb(dev->base + DEV_OFF_CMD, 0x30);
    if (wait_disk_ready(dev)) return write_disk(dev,buffer,lba,sectors);
    uint16_t* buffer16 = (uint16_t*)buffer;
    for (uint16_t i = 0; i < sectors; i++) {
        if (wait_disk_ready(dev)) return write_disk(dev,buffer,lba,sectors);
        for (uint16_t j = 0; j < 256; j++) {
            outw(dev->base + DEV_OFF_DATA,*buffer16);
            disk_delay(dev);
            buffer16++;
        }
    }
}

void print_partition(partition_t* partition) {
    if (partition->type == 0) {
        print("    Free");
        return;
    }
    print("    Attributes: ");
    print_hex(partition->attributes);
    print("\n\r    LBA: ");
    print_hex(partition->lba);
    print("\n\r    Type: ");
    print_hex(partition->type);
    printf("\n\r    Sectors: %x", partition->sectors);
}

void print_mbr(mbr_t* mbr) {
    print("UID: ");
    print_hex(mbr->uid);
    print("\n\rPartition 0: \n\r");
    print_partition(&mbr->partition0);
    print("\n\rPartition 1: \n\r");
    print_partition(&mbr->partition1);
    print("\n\rPartition 2: \n\r");
    print_partition(&mbr->partition2);
    print("\n\rPartition 3: \n\r");
    print_partition(&mbr->partition3);
    printf("\n\rValid: %s\n\r", (mbr->signature == 0xAA55)?"Yes":"No");
}

device_t* devdup(device_t* dev) {
    device_t* dup = (device_t*)heap::malloc(sizeof(device_t));
    memcpy(dup,dev,sizeof(device_t));
    return dup;
}