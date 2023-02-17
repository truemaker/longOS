#include <font.h>
#include <vga.h>
#include <serial.h>

namespace font {
    uint8_t* parse_font(uint8_t* font) {
        uint8_t* vga_font = (uint8_t*)heap::malloc(0x1000);
        font_header_t* hdr = (font_header_t*)font;
        serial::write_serial("Loading Font\n\r",14);
        if (hdr->signature[0] != 'r') {serial::write_serial("RVF Loader: Invalid header0\n\r",28);for (;;);return NULL;}
        if (hdr->signature[1] != 'f') {serial::write_serial("RVF Loader: Invalid header1\n\r",28);for (;;);return NULL;}
        if (hdr->signature[2] != 'n') {serial::write_serial("RVF Loader: Invalid header2\n\r",28);for (;;);return NULL;}
        if (hdr->signature[3] != 't') {serial::write_serial("RVF Loader: Invalid header3\n\r",28);for (;;);return NULL;}
        if (hdr->width != 8) {serial::write_serial("RVF Loader: Incompatible font size\n\r",36);for (;;);return NULL;}
        print("Starting relocation...\n\r");
        for (uint8_t i = 0; i < hdr->relocations; i++) {
            font_relocation_t* reloc = (font_relocation_t*)(font + sizeof(font_header_t) + hdr->height*hdr->chars)+i;
            printf("Relocating %x->%x\n\r",reloc->src_index,reloc->dst_index);
            memcpy(vga_font+(reloc->dst_index*32),(font + sizeof(font_header_t) + hdr->height*reloc->src_index),hdr->height);
        }
        return vga_font;
    }
}