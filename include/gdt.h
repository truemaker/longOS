#pragma once
#include <typedef.h>
typedef struct gdt_descriptor {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdt_descriptor_t;

typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags_limit_hi;
    uint8_t base_hi;
}__attribute__((packed)) gdt_entry_t;

typedef struct gdt {
    gdt_entry_t null; //0x00
    gdt_entry_t kernel_code; //0x08
    gdt_entry_t kernel_data; //0x10
    gdt_entry_t user_null;
    gdt_entry_t user_code;
    gdt_entry_t user_data;
} __attribute__((packed)) 
__attribute((aligned(0x1000))) gdt_t;

typedef enum
{
    GDT_ACCESS_CODE_READABLE                = 0x02,
    GDT_ACCESS_DATA_WRITEABLE               = 0x02,

    GDT_ACCESS_CODE_CONFORMING              = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

    GDT_ACCESS_DATA_SEGMENT                 = 0x10,
    GDT_ACCESS_CODE_SEGMENT                 = 0x18,

    GDT_ACCESS_DESCRIPTOR_LDT               = 0x00,
    GDT_ACCESS_DESCRIPTOR_TSS               = 0x01,

    GDT_ACCESS_RING0                        = 0x00,
    GDT_ACCESS_RING1                        = 0x20,
    GDT_ACCESS_RING2                        = 0x40,
    GDT_ACCESS_RING3                        = 0x60,

    GDT_ACCESS_PRESENT                      = 0x80,

} GDT_ACCESS;

typedef enum 
{
    GDT_FLAG_64BIT                          = 0x20,
    GDT_FLAG_32BIT                          = 0x40,
    GDT_FLAG_16BIT                          = 0x00,

    GDT_FLAG_GRANULARITY_1B                 = 0x00,
    GDT_FLAG_GRANULARITY_4K                 = 0x80,
} GDT_FLAGS;

extern gdt_t default_gdt;

extern "C" void load_gdt(gdt_descriptor_t* gdt_descriptor_t);
void init_gdt();