#include <convert.h>

uint8_t bcd2bin(uint8_t bcd) {
    return (bcd & 0x0f) + ((bcd / 16)*10);
}