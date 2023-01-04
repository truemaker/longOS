#pragma once
#include <typedef.h>
struct idt64 {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t types_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_desc64 {
    uint16_t limit;
    uint64_t address;
} __attribute__((packed));

struct interrupt_frame {
    uint64_t err_code;
    uint64_t rip;
    uint32_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint32_t ss;
} __attribute__((packed));

typedef struct idt64 idt64_t;
typedef struct idt_desc64 idt_desc64_t;
typedef struct interrupt_frame interrupt_frame_t;

extern idt64_t _idt[256];
extern uint64_t isr1;
extern uint64_t pf;
extern idt_desc64_t idt_desc;
void init_idt();
void register_interrupt(uint64_t id, uint64_t addr);
__attribute__((interrupt)) void pagef_handler(interrupt_frame_t* int_frame);
__attribute__((interrupt)) void doublef_handler(interrupt_frame_t* int_frame);
__attribute__((interrupt)) void gpf_handler(interrupt_frame_t* int_frame);