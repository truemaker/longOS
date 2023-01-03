#pragma once
#include <typedef.h>
#include <idt.h>
uint8_t far_peekb(uint16_t sel, void* offset);
uint16_t far_peekw(uint16_t sel, void* offset);
uint32_t far_peekl(uint16_t sel, void* offset);
void far_pokeb(uint16_t sel, void* offset, uint8_t val);
void far_pokew(uint16_t sel, void* offset, uint16_t val);
void far_pokel(uint16_t sel, void* offset, uint32_t val);
bool are_interrupts_enabled();
uint64_t save_irqdisable();
void irq_restore(uint64_t flags);
void lidt(void* base, uint16_t size);
void lidt(idt_desc64_t desc);
void cpuid(int code, uint32_t* a, uint32_t* d);
uint64_t rdtsc();
uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
uint64_t read_cr4();
void invlpg(void* m);
void wrmsr(uint64_t msr, uint64_t val);
uint64_t rdmsr(uint64_t msr);
void reload_cr3();