#include <bitmap.h>
bool bitmap_t::operator[](uint64_t index) {
    if (index > size) return false;
    uint64_t idx4baligned = index / 32;
    uint64_t idxin4b = index % 32;
    uint32_t b = bytes[idx4baligned];
    uint32_t mask = (0b10000000000000000000000000000000 >> idxin4b);
    if (b & mask) return true;
    return false;
}

void bitmap_t::set(uint64_t index, bool value) {
    if (index > size) return;
    uint64_t idx4baligned = index / 32;
    uint64_t idxin4b = index % 32;
    uint32_t b = bytes[idx4baligned];
    uint32_t mask = (0b10000000000000000000000000000000 >> idxin4b);
    b &= ~mask;
    if (value) b |= mask;
    bytes[idx4baligned] = b;
}