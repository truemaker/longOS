#pragma once
#include <typedef.h>

#define CENTURY 21

namespace CMOS {
    uint8_t get_register(uint8_t reg);
    void write_register(uint8_t reg, uint8_t val);
    uint8_t get_drive_types();
    uint16_t get_memory_size();
    namespace RTC {
        void wait_update(void); // May take up to 1 second
        uint8_t get_second();
        uint8_t get_minute();
        uint8_t get_hour();
        uint8_t get_day();
        uint8_t get_month();
        uint8_t get_year();
        uint8_t get_century();
        void print_time();
    }
}