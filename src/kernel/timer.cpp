#include <timer.h>
#include <io.h>
#include <vga.h>
#include <idt.h>

namespace PIT {
    uint16_t divisor = 65535;
    uint64_t millis_since_boot;
    extern "C" void timer_handler() {
        millis_since_boot += 1;
        outb(0x20,0x20);
        outb(0xa0, 0x20);
    }

    void init_timer() {
        register_interrupt(32, (uint64_t)&timer);
        set_frequency(1000);
        outb(0x21, inb(0x21) & ~0b00000001);
        outb(0xa1, inb(0xa1));
    }

    void sleep(uint64_t milliseconds) {
        uint64_t start_time = millis_since_boot;
        while (millis_since_boot < start_time + milliseconds) {
            asm("hlt");
        }
    }

    void set_divisor(uint64_t new_divisor) {
        if (new_divisor < 100) new_divisor = 100;
        divisor = new_divisor;
        outb(0x40, (uint8_t)(new_divisor & 0x00ff));
        io_wait();
        outb(0x40, (uint8_t)((new_divisor & 0xff00) >> 8));
    }

    uint64_t get_frequency() {
        return base_frequency / divisor;
    }

    void set_frequency(uint64_t frequency) {
        set_divisor(base_frequency / frequency);
    }
}