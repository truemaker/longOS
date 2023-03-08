#include <cmos.h>
#include <io.h>
#include <convert.h>
#include <string.h>
#include <vga.h>
#include <heap.h>

namespace CMOS {
    uint8_t get_register(uint8_t reg) {
        outb(0x70,0x80 | reg);
        return inb(0x71);
    }

    void write_register(uint8_t reg, uint8_t val) {
        outb(0x70,0x80 | reg);
        outb(0x71, val);
    }

    uint8_t get_drive_types() {
        return get_register(0x10);
    }

    uint16_t get_memory_size() {
        uint16_t total;
        uint8_t low, high;
        low = get_register(0x30);
        high = get_register(0x31);
        return (low << 16) | high;
    }
    namespace RTC {
        void wait_update() {
            while (get_register(0xA) & 0x80) asm("nop");
        }

        uint8_t get_second() { return get_register(0x00); }
        uint8_t get_minute() { return get_register(0x02); }
        uint8_t get_hour() { return get_register(0x04); }
        uint8_t get_day() { return get_register(0x07); }
        uint8_t get_month() { return get_register(0x08); }
        uint8_t get_year() { return get_register(0x09); }
        uint8_t get_century() { return get_register(0x32); }

        void print_time() {
            uint8_t sb = get_register(0xB);
            uint8_t s,m,h,d,M,y,c;
            wait_update();
            s = get_second();
            m = get_minute();
            h = get_hour();
            d = get_day();
            M = get_month();
            y = get_year();
            c = get_century();
            uint8_t ampm = h & 0x80;
            if (!(0x04 & sb)) {
                s = bcd2bin(s);
                m = bcd2bin(m);
                h = bcd2bin(h);
                d = bcd2bin(d);
                M = bcd2bin(M);
                y = bcd2bin(y);
                c = bcd2bin(c);
            }
            if (!(0x02 & sb) && ampm) {
                h = ((h & 0x7f) + 12) % 24;
            }
            uint64_t year = y;
            if (c != 0) {
                year += c * 100;
            } else {
                year += (CENTURY - 1) * 100;
            }
            char* minute;
            char* second;
            char* day;
            char* month;
            char* ystring;
            char* hour;
            minute = uitos(m,10,2);
            second = uitos(s,10,2);
            day = uitos(d,10,2);
            month = uitos(M,10,2);
            ystring = uitos(year,10,4);
            hour = uitos(h,10);
            printf("%s:%s:%s %s.%s.%s",hour,minute,second,day,month,ystring);
            heap::free(minute);
            heap::free(second);
            heap::free(day);
            heap::free(month);
            heap::free(ystring);
            heap::free(hour);
        }
    }
}