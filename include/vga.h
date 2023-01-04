#pragma once
#include <typedef.h>
#define VGA_MEM (uint8_t*)0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum foreground_color {
    FG_BLACK           = 0x00,
    FG_BLUE            = 0x01,
    FG_GREEN           = 0x02,
    FG_CYAN            = 0x03,
    FG_RED             = 0x04,
    FG_MAGENTA         = 0x05,
    FG_BROWN           = 0x06,
    FG_LIGHTGRAY       = 0x07,
    FG_DARKGRAY        = 0x08,
    FG_LIGHTBLUE       = 0x09,
    FG_LIGHTGREEN      = 0x0a,
    FG_LIGHTCYAN       = 0x0b,
    FG_LIGHTRED        = 0x0c,
    FG_LIGHTMAGENTA    = 0x0d,
    FG_YELLOW          = 0x0e,
    FG_WHITE           = 0x0f
};

enum background_color {
    BG_BLACK           = 0x00,
    BG_BLUE            = 0x10,
    BG_GREEN           = 0x20,
    BG_CYAN            = 0x30,
    BG_RED             = 0x40,
    BG_MAGENTA         = 0x50,
    BG_BROWN           = 0x60,
    BG_LIGHTGRAY       = 0x70,
    BG_DARKGRAY        = 0x80,
    BG_LIGHTBLUE       = 0x90,
    BG_LIGHTGREEN      = 0xa0,
    BG_LIGHTCYAN       = 0xb0,
    BG_LIGHTRED        = 0xc0,
    BG_LIGHTMAGENTA    = 0xd0,
    BG_YELLOW          = 0xe0,
    BG_WHITE           = 0xf0
};

void set_cursor_pos(uint16_t pos);
uint16_t coord_from_pos(uint16_t x, uint16_t y);
void print(const char *str);
void printc(char c);
void printf(const char *str, ...);
uint8_t attribute(uint8_t fc, uint8_t bc);
void clear(uint8_t color = BG_BLUE | FG_WHITE);
void print_hex(uint16_t word);