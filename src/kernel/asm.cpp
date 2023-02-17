#include <asm.h>
uint8_t far_peekb(uint16_t sel, void* offset) {
    uint8_t ret;
    asm ( "push %%fs\n\t"
          "mov %1, %%fs\n\t"
          "mov %%fs:(%2), %0\n\t"
          "pop %%fs"
          : "=r"(ret) : "g"(sel), "r"(offset)
    );
    return ret;
}

uint16_t far_peekw(uint16_t sel, void* offset) {
    uint16_t ret;
    asm ( "push %%fs\n\t"
          "mov %1, %%fs\n\t"
          "mov %%fs:(%2), %0\n\t"
          "pop %%fs"
          : "=r"(ret) : "g"(sel), "r"(offset)
    );
    return ret;
}

uint32_t far_peekl(uint16_t sel, void* offset) {
    uint32_t ret;
    asm ( "push %%fs\n\t"
          "mov %1, %%fs\n\t"
          "mov %%fs:(%2), %0\n\t"
          "pop %%fs"
          : "=r"(ret) : "g"(sel), "r"(offset)
    );
    return ret;
}

void far_pokeb(uint16_t sel, void* offset, uint8_t val) {
    asm ( "push %%fs\n\t"
          "mov  %0, %%fs\n\t"
          "movb %2, %%fs:(%1)\n\t"
          "pop %%fs"
          : : "g"(sel), "r"(offset), "r"(val) );
}

void far_pokew(uint16_t sel, void* offset, uint16_t val) {
    asm ( "push %%fs\n\t"
          "mov  %0, %%fs\n\t"
          "movw %2, %%fs:(%1)\n\t"
          "pop %%fs"
          : : "g"(sel), "r"(offset), "r"(val) );
}

void far_pokel(uint16_t sel, void* offset, uint32_t val) {
    asm ( "push %%fs\n\t"
          "mov  %0, %%fs\n\t"
          "movl %2, %%fs:(%1)\n\t"
          "pop %%fs"
          : : "g"(sel), "r"(offset), "r"(val) );
}

bool are_interrupts_enabled() {
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}

uint64_t save_irqdisable() {
    unsigned long flags;
    asm volatile ("pushf\n\tcli\n\tpop %0" : "=r"(flags) : : "memory");
    return flags;
}

void irqrestore(uint64_t flags) {
    asm ("push %0\n\tpopf" : : "rm"(flags) : "memory","cc");
}

void lidt(void* base, uint16_t size)
{   // This function works in 32 and 64bit mode
    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };
    asm ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}

void lidt(idt_desc64_t desc) {
    asm ( "lidt %0" : : "m"(desc) );
}

void cpuid(int code, uint32_t* a, uint32_t* d) {
    asm volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
}

uint64_t rdtsc() {
    uint32_t low, high;
    asm volatile("rdtsc":"=a"(low),"=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t read_cr0() {
    uint64_t val;
    asm volatile( "mov %%cr0, %0" : "=r"(val) );
    return val;
}

uint64_t read_cr2() {
    uint64_t val;
    asm volatile( "mov %%cr2, %0" : "=r"(val) );
    return val;
}

uint64_t read_cr3() {
    uint64_t val;
    asm volatile( "mov %%cr3, %0" : "=r"(val) );
    return val;
}

uint64_t read_cr4() {
    uint64_t val;
    asm volatile( "mov %%cr4, %0" : "=r"(val) );
    return val;
}

void invlpg(void* m) {
    asm volatile("invlpg (%0)" : : "b"(m) : "memory");
}

void wrmsr(uint64_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
	uint32_t high = value >> 32;
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "a"(low), "d"(high)
	);
}

uint64_t rdmsr(uint64_t msr) {
    uint32_t low, high;
	asm volatile (
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(msr)
	);
	return ((uint64_t)high << 32) | low;
}

void reload_cr3() {
    asm("mov %0, %%cr3" :: "r"(read_cr3()));
}