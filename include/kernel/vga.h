#pragma once
#include <typedef.h>
#define VGA_MEM (uint8_t*)0xb8000
#define VGA_WIDTH w
//40
//80
#define VGA_HEIGHT h
//25
//25

/* Attribute Controller Registers */
#define	GRAPHICS_AC_INDEX 0x3C0
#define	GRAPHICS_AC_READ 0x3C1
#define	GRAPHICS_AC_WRITE 0x3C0

/*
Miscellaneous Output
*/
#define	GRAPHICS_MISC_READ 0x3CC
#define	GRAPHICS_MISC_WRITE 0x3C2

/* Sequencer Registers */
#define GRAPHICS_SEQ_INDEX 0x3C4
#define GRAPHICS_SEQ_DATA 0x3C5

/* GRAPHICS Color Palette Registers */
#define	GRAPHICS_DAC_READ_INDEX 0x3C7
#define	GRAPHICS_DAC_WRITE_INDEX 0x3C8
#define	GRAPHICS_DAC_DATA 0x3C9

/* Graphics Controller Registers */
#define GRAPHICS_GC_INDEX 0x3CE
#define GRAPHICS_GC_DATA 0x3CF

/* CRT Controller Registers */
#define GRAPHICS_CRTC_INDEX 0x3D4
#define GRAPHICS_CRTC_DATA 0x3D5

/* General Control and Status Registers */
#define	GRAPHICS_INSTAT_READ 0x3DA

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

extern uint64_t w,h;

void set_cursor_pos(uint16_t pos);
uint16_t coord_from_pos(uint16_t x, uint16_t y);
void print(const char *str);
void printc(char c);
void printf(const char *str, ...);
void debugf(const char *str, ...);
uint8_t attribute(uint8_t fc, uint8_t bc);
void clear(uint8_t color = BG_BLUE | FG_WHITE);
void print_hex(uint8_t byte);
void print_hex(uint16_t word);
void print_hex(uint32_t dword);
void print_hex(uint64_t quad);
void init_vga(void);
void set_mode(uint64_t w, uint64_t h);
void print_trans(void);
void set_line_color(uint64_t line, uint8_t c, uint64_t start=0, uint64_t end=VGA_WIDTH);
void disable_cursor(void);
void enable_cursor(void);
extern void write_font(unsigned char *buf, unsigned font_height);
void clear_line(void);