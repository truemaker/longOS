#include <bitmap.h>
#include <vga.h>
bool bitmap_t::operator[](uint64_t index) {
    return get(index);
}

bool bitmap_t::get(uint64_t index) {
    debugf("Getting %x\n\r",index);
    if (index > size * 8) {printf("[BITMAP] Bitmap out of bounds for read: %x\n\r",index); return false;}
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;
    uint8_t bit_indexer = 0b10000000 >> bit_index;
    if (bytes[byte_index]&bit_indexer) return true;
    return false;
}

void bitmap_t::set(uint64_t index, bool value) {
    debugf("Setting %x\n\r",index);
    if (index > size * 8) {printf("[BITMAP] Bitmap out of bounds for write: %x\n\r",index); return;}
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;
    uint8_t bit_indexer = 0b10000000 >> bit_index;
    bytes[byte_index] |= bit_indexer;
}