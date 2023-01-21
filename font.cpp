#include <font.h>
#include <vga.h>
#include <serial.h>

uint8_t* parse_font(uint8_t* font) {
    uint8_t* vga_font = (uint8_t*)malloc(0x1000);
    font_header_t* hdr = (font_header_t*)font;
    write_serial("Loading Font\n\r",14);
    if (hdr->signature[0] != 'r') {write_serial("RVF Loader: Invalid header\n\r",28);return NULL;}
    if (hdr->signature[1] != 'f') {write_serial("RVF Loader: Invalid header\n\r",28);return NULL;}
    if (hdr->signature[2] != 'n') {write_serial("RVF Loader: Invalid header\n\r",28);return NULL;}
    if (hdr->signature[3] != 't') {write_serial("RVF Loader: Invalid header\n\r",28);return NULL;}
    
    return vga_font;
}