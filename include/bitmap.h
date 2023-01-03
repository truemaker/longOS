#pragma once
#include <typedef.h>
typedef struct bitmap {
    uint32_t* bytes;
    uint64_t size;
    bool operator[](uint64_t index);
    void set(uint64_t index, bool value);
} bitmap_t;