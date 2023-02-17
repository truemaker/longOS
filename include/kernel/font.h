#pragma once
#include <typedef.h>
#include <memory.h>
#include <heap.h>
namespace font {
    typedef struct font_header {
        uint8_t signature[4];
        uint8_t width;
        uint8_t height;
        uint8_t chars;
    } font_header_t;
    
    typedef struct font_relocation {
        uint8_t src_index;
        uint8_t dst_index;
    } font_relocation_t;
    
    uint8_t* parse_font(uint8_t*);
}