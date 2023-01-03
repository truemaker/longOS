#include <idt.h>
#include <vga.h>
#include <io.h>
#include <asm.h>

void pic_remap() {
    uint8_t a1,a2;
    a1 = inb(0x21);
    a2 = inb(0xA1);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0);
    outb(0xa1, 8);
    outb(0x21, 4);
    outb(0xa1, 2);
    outb(0x21, 1);
    outb(0xa1, 1);
    outb(0x21, a1);
    outb(0xa1, a2);
}

void init_idt() {
    _idt[1].zero = 0;
    _idt[1].offset_low = (uint16_t)(((uint64_t)&isr1 & 0xffff));
    _idt[1].offset_mid = (uint16_t)(((uint64_t)&isr1 >> 16) & 0xffff);
    _idt[1].offset_high = (uint32_t)(((uint64_t)&isr1 >> 32) & 0xffffffff);
    _idt[1].ist = 0;
    _idt[1].selector = 0x08;
    _idt[1].types_attr = 0x8e;

    _idt[0xe].zero = 0;
    _idt[0xe].offset_low = (uint16_t)(((uint64_t)&pagef_handler & 0xffff));
    _idt[0xe].offset_mid = (uint16_t)(((uint64_t)&pagef_handler >> 16) & 0xffff);
    _idt[0xe].offset_high = (uint32_t)(((uint64_t)&pagef_handler >> 32) & 0xffffffff);
    _idt[0xe].ist = 0;
    _idt[0xe].selector = 0x08;
    _idt[0xe].types_attr = 0x8e;

    _idt[0x8].zero = 0;
    _idt[0x8].offset_low = (uint16_t)(((uint64_t)&doublef_handler & 0xffff));
    _idt[0x8].offset_mid = (uint16_t)(((uint64_t)&doublef_handler >> 16) & 0xffff);
    _idt[0x8].offset_high = (uint32_t)(((uint64_t)&doublef_handler >> 32) & 0xffffffff);
    _idt[0x8].ist = 0;
    _idt[0x8].selector = 0x08;
    _idt[0x8].types_attr = 0x8e;

    _idt[0xd].zero = 0;
    _idt[0xd].offset_low = (uint16_t)(((uint64_t)&gpf_handler & 0xffff));
    _idt[0xd].offset_mid = (uint16_t)(((uint64_t)&gpf_handler >> 16) & 0xffff);
    _idt[0xd].offset_high = (uint32_t)(((uint64_t)&gpf_handler >> 32) & 0xffffffff);
    _idt[0xd].ist = 0;
    _idt[0xd].selector = 0x08;
    _idt[0xd].types_attr = 0x8e;

    pic_remap();
    outb(0x21, 0xfd);
    outb(0xa1, 0xff);
    lidt(idt_desc);
    asm("sti");
}

extern "C" void isr1_handler() {
    uint8_t c = inb(0x60);
    print("Interrupt\n\r");
    outb(0x20,0x20);
    outb(0xa0, 0x20);
}

__attribute__((interrupt)) void pagef_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    printf("A page fault has occured\n\rFault address: %h\n\rPML4: %h\n\rRIP: %h", read_cr2(), read_cr3(),int_frame->rip);
    for (;;);
}

__attribute__((interrupt)) void doublef_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    print("A double fault has occured");
    for (;;);
}

__attribute__((interrupt)) void gpf_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    printf("A general protection fault has occured\n\rCS: %h\n\rRIP: %h\n\r%t",int_frame->cs,int_frame->rip);
    for (;;);
}