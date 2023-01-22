#pragma once
#include <typedef.h>

namespace PIT {
    extern uint64_t millis_since_boot;
    const uint64_t base_frequency = 1193182;
    void init_timer();
    void sleep(uint64_t milliseconds);
    void set_divisor(uint64_t divisor);
    uint64_t get_frequency();
    void set_frequency(uint64_t frequency);
}