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

void register_interrupt(uint64_t id, uint64_t addr) {
    _idt[id].zero = 0;
    _idt[id].offset_low = (uint16_t)((addr & 0xffff));
    _idt[id].offset_mid = (uint16_t)((addr >> 16) & 0xffff);
    _idt[id].offset_high = (uint32_t)((addr >> 32) & 0xffffffff);
    _idt[id].ist = 0;
    _idt[id].selector = 0x08;
    _idt[id].types_attr = 0x8e;
}

void init_idt() {
    register_interrupt(1, (uint64_t)&isr1);
    register_interrupt(0xe, (uint64_t)&pagef_handler);
    register_interrupt(0x8, (uint64_t)&doublef_handler);
    register_interrupt(0xd, (uint64_t)&gpf_handler);

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