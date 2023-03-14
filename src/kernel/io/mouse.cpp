#include <mouse.h>
#include <vga.h>
#include <idt.h>
#include <io.h>

void mouse_wait() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if (inb(0x64) & 0b10) {
            return;
        }
    }
}

void mouse_wait_input() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if (inb(0x64) & 0b1) {
            return;
        }
    }
}

void mouse_write(uint8_t val) {
    mouse_wait();
    outb(0x64, 0xD4);
    mouse_wait();
    outb(0x60, val);
}

uint8_t mouse_read() {
    mouse_wait_input();
    return inb(0x60);
}

void kb_send(uint8_t cmd) {
    mouse_wait();
    outb(0x64,cmd);
}

void enable_mouse() {
    kb_send(0x20);
    uint8_t status = inb(0x60);
    status |= 0x02;
    status &= ~0x20;
    kb_send(0x60);
    outb(0x60,status);
    mouse_write(0xFF);
    mouse_wait();
    while (mouse_read() != 0xFA);
    while (mouse_read() != 0xAA);
    mouse_write(0xF4);
    while (mouse_read() != 0xFA);
    mouse_wait();
}

void init_mouse() {
    enable_mouse();
    outb(0x21, inb(0x21));
    outb(0xa1, inb(0xa1) & ~0b00010000);
    print("Finished mouse init");
}