#include <typedef.h>
void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t returnVal;
    asm volatile("inb %1, %0"
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

uint16_t inw(uint16_t port) {
    uint16_t returnVal;
    asm volatile("inw %1, %0"
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

uint32_t inl(uint16_t port) {
    uint32_t returnVal;
    asm volatile("inl %1, %0"
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

void io_wait() {
    outb(0x80, 0);
}