#include <idt.h>
#include <vga.h>
#include <io.h>
#include <asm.h>
#include <memory.h>
#include <serial.h>
#include <debug.h>


void(*main_keyboard_handler)(uint8_t scan_code);
uint64_t nested_errors = 0;

void pic_remap(void) {
    uint8_t a1,a2;
    a1 = inb(0x21);
    a2 = inb(0xA1);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 32);
    outb(0xa1, 40);
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

void init_idt(void) {
    register_interrupt(33, (uint64_t)&isr1);
    register_interrupt(0xe, (uint64_t)&pagef_handler);
    register_interrupt(0x8, (uint64_t)&doublef_handler);
    register_interrupt(0xd, (uint64_t)&gpf_handler);
    register_interrupt(0xc, (uint64_t)&ssf_handler);

    pic_remap();
    outb(0x21, 0b11111101);
    outb(0xa1, 0b11111111);
    lidt(idt_desc);
    asm("sti");
}

extern "C" void isr1_handler(void) {
    uint8_t c = inb(0x60);
    if (main_keyboard_handler) main_keyboard_handler(c);
    outb(0x20,0x20);
    outb(0xa0, 0x20);
}

__attribute__((interrupt)) void pagef_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    if (nested_errors) for (;;);
    nested_errors++;
    printf("A page fault has occured\n\rFault address: %h\n\rPML4: %h\n\rRIP: %h", read_cr2(), read_cr3(),int_frame->rip);
    print("\n\rReadable Message: ");
    uint64_t err = int_frame->err_code;
    if (err & 0x4) print("User");
    else print("Kernel");
    print(" tried to ");
    if (err & 0x2) print("write to");
    else print("read from");
    print(" a page that is ");
    if (err & 1) print("present");
    else print("not present");
    print(" during ");
    if ((1 << 4)&int_frame->err_code) print("instruction-fetch");
    else print("regular execution");
    print(" due to ");
    if ((1 << 6)&int_frame->err_code) print("shadow-stack access");
    else if ((1 << 5)&int_frame->err_code) print("protection-key");
    else print("bad programming");
    new_line();
    trace(10,(struct stackframe*)int_frame->rsp);
    for (;;);
}

__attribute__((interrupt)) void doublef_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    print("A double fault has occured");
    for (;;);
}

__attribute__((interrupt)) void ssf_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    print("A stack-segment fault has occured!");
    for (;;);
}

__attribute__((interrupt)) void gpf_handler(interrupt_frame_t* int_frame) {
    asm("cli");
    if (nested_errors) for (;;);
    nested_errors++;
    printf("A general protection fault has occured\n\rCS: %h\n\rRIP: %h\n\rRSP: %h\n\r",int_frame->cs,int_frame->rip,int_frame->rsp >> 32);
    trace(10,(struct stackframe*)__builtin_frame_address(0));
    print("Readable Message: ");
    if (int_frame->err_code == 0) print("Not Segment Related");
    else {
        print("Segment error was caused ");
        if (int_frame->err_code & 1) print("externally");
        else {
            print("internally\n\rIt occured in the ");
            switch ((0b110 & int_frame->err_code) >> 1) {
                case 0: print("GDT");break;
                case 1: print("IDT");break;
                case 2: print("LDT");break;
                case 3: print("IDT");break;
                default: print("???");
            }
            print(" at selector ");
            print_hex((0xfff8 & int_frame->err_code) >> 3);
        }
    }
    new_line();
    for (;;);
}