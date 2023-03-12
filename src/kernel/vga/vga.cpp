#include <io.h>
#include <vga.h>
#include <typedef.h>
#include <stdarg.h>
#include <debug.h>
#include <memory.h>
#include <defines.h>
#include <serial.h>
#include <timer.h>
#include <maths.h>
#include "vga_font.cpp"
#include "vga_graphics.cpp"

uint16_t vga_pos = 0;
uint64_t w = 0;
uint64_t h = 0;
bool g = false;
uint8_t b = 0;

unsigned char g_80x25_text[] =
{
/* MISC */
	0x67,
/* SEQ */
	0x03, 0x00, 0x03, 0x00, 0x02,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
	0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00
};

unsigned char g_90x30_text[] {
/* MISC */
	0xE7,
/* SEQ */
	0x03, 0x01, 0x03, 0x00, 0x02,
/* CRTC */
	0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x2D, 0x10, 0xE8, 0x05, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};


unsigned char g_90x60_text[] =
{
/* MISC */
	0xE7,
/* SEQ */
	0x03, 0x01, 0x03, 0x00, 0x02,
/* CRTC */
	0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
	0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};

unsigned char g_320x200x256[] =
{
/* MISC */
	0x63,
/* SEQ */
	0x03, 0x01, 0x0F, 0x00, 0x0E,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9C, 0x0E, 0x8F, 0x28,	0x40, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, 0x00, 0x0F, 0x00,	0x00
};

unsigned char g_640x480x16[] =
{
/* MISC */
	0xE3,
/* SEQ */
	0x03, 0x01, 0x08, 0x00, 0x06,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x01, 0x00, 0x0F, 0x00, 0x00
}; // 640x480 16 color planar framebuffer at 0xA0000

void load_registers(uint8_t* regs) {
    unsigned i;
/* write MISCELLANEOUS reg */
	outb(GRAPHICS_MISC_WRITE, *regs);
	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < 5; i++)
	{
		outb(GRAPHICS_SEQ_INDEX, i);
		outb(GRAPHICS_SEQ_DATA, *regs);
		regs++;
	}
/* unlock CRTC registers */
	outb(GRAPHICS_CRTC_INDEX, 0x03);
	outb(GRAPHICS_CRTC_DATA, inb(GRAPHICS_CRTC_DATA) | 0x80);
	outb(GRAPHICS_CRTC_INDEX, 0x11);
	outb(GRAPHICS_CRTC_DATA, inb(GRAPHICS_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	for(i = 0; i < 25; i++)
	{
		outb(GRAPHICS_CRTC_INDEX, i);
		outb(GRAPHICS_CRTC_DATA, *regs);
		regs++;
	}
/* write GRAPHICS CONTROLLER regs */
	for(i = 0; i < 9; i++)
	{
		outb(GRAPHICS_GC_INDEX, i);
		outb(GRAPHICS_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < 20; i++)
	{
		(void)inb(GRAPHICS_INSTAT_READ);
		outb(GRAPHICS_AC_INDEX, i);
		outb(GRAPHICS_AC_WRITE, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inb(GRAPHICS_INSTAT_READ);
	outb(GRAPHICS_AC_INDEX, 0x20);
}

void print_trans(void) {
    load_registers(g_640x480x16);
    gclear(0);
    uint64_t offset = 220;
    for (uint64_t i = 0; i<240;i++) {
        uint64_t j;
        for (j = 0; j<40;j++) {
            write_pixel4p(j+offset,i,FG_LIGHTBLUE);
        }
        for (; j<80;j++) {
            write_pixel4p(j+offset,i,FG_LIGHTRED);
        }
        for (; j<120;j++) {
            write_pixel4p(j+offset,i,FG_WHITE);
        }
        for (; j<160;j++) {
            write_pixel4p(j+offset,i,FG_LIGHTRED);
        }
        for (; j<200;j++) {
            write_pixel4p(j+offset,i,FG_LIGHTBLUE);
        }
    }
    for (uint64_t i = 240; i<480;i++) {
        uint64_t j;
        for (j = 0; j<50;j++) {
            write_pixel4p(j+offset,i,FG_YELLOW);
        }
        for (; j<100;j++) {
            write_pixel4p(j+offset,i,FG_WHITE);
        }
        for (; j<150;j++) {
            write_pixel4p(j+offset,i,FG_MAGENTA);
        }
        for (; j<200;j++) {
            write_pixel4p(j+offset,i,FG_BLACK);
        }
    }
    write_screen();
}

void enter_graphics() {
    load_registers(g_640x480x16);
    gclear();
    write_screen();
    g = true;
    w = 80;
    h = 30;
    set_cursor_pos(0);
}

void test_graphics() {
    load_registers(g_640x480x16);
    gclear();
    write_screen();
    draw_font(0,0, 1,g_8x16_font,16,0x1);
    draw_font(0,0, 'H',g_8x16_font,16,0x1);
    draw_font(8,0, 'e',g_8x16_font,16,0x1);
    draw_font(16,0,'l',g_8x16_font,16,0x1);
    draw_font(24,0,'l',g_8x16_font,16,0x1);
    draw_font(32,0,'o',g_8x16_font,16,0x1);
    draw_font(48,0,'W',g_8x16_font,16,0x1);
    draw_font(56,0,'o',g_8x16_font,16,0x1);
    draw_font(64,0,'r',g_8x16_font,16,0x1);
    draw_font(72,0,'l',g_8x16_font,16,0x1);
    draw_font(80,0,'d',g_8x16_font,16,0x1);
    draw_font(88,0,'!',g_8x16_font,16,0x1);
    write_screen();
    draw_font(96,0,1,g_8x16_font,16,0x1);
    write_screen();
    for (uint64_t x = 0; x < 3; x++) {
        for (uint64_t y = 0; y < 3; y++) {
            draw_line(30+x*5-5,30+y*5-5,30-x*5-5,30-y*5-5,0xff);
            draw_line(30-x*5-5,30+y*5-5,30+x*5-5,30-y*5-5,0xff);
        }
    }
    write_screen();
    draw_circle(400,300,32,0xff);
    write_screen();
    draw_ellipse(100,200,32,16,0xff);
    write_screen();
    draw_rect(200,100,64,64,0xff);
    fill_rect(216,116,32,32,0xff);
    write_screen();
    draw_font_transparent(228,124,1,g_8x16_font,16,0x1);
    write_screen();
}

void set_mode(uint64_t wd, uint64_t ht) {
    switch (wd) {
        case 80:
            switch (ht) {
                case 25: load_registers(g_80x25_text); write_font(g_8x16_font,16); break;
                default: load_registers(g_80x25_text); write_font(g_8x16_font,16); w = 80; h = 25; clear(); printf("[VGA] No mode with height %x.",ht); for (;;);
            } break;
        case 90:
            switch (ht) {
                case 30: load_registers(g_90x30_text); write_font(g_8x16_font,16); break;
                case 60: load_registers(g_90x60_text); write_font(g_8x8_font,8); break;
                default: load_registers(g_80x25_text); write_font(g_8x16_font,16); w = 80; h = 25; clear(); printf("[VGA] No mode with height %x.",ht); for (;;);
            } break;
        default: load_registers(g_80x25_text); write_font(g_8x16_font,16); w = 80; h = 25; clear(); printf("[VGA] No mode with width %x.",wd); for (;;);
    }
    w = wd; h = ht;
    clear();
}

void init_vga(void) {
    g = false;
    set_mode(90,30);
#ifdef GRAPHICS_TEST
    test_graphics();
    //print_trans();
    for (;;);
#endif
}

void clear_down_row(void) {
    memset(VGA_MEM+VGA_WIDTH*(VGA_HEIGHT-1)*2,attribute(FG_WHITE,BG_BLUE),VGA_WIDTH*2);
    for (uint64_t i = 0; i < VGA_WIDTH; i++) {
        *(VGA_MEM+VGA_WIDTH*(VGA_HEIGHT-1)*2+i*2) = 0;
    }
}

void set_cursor_pos(uint16_t pos) {
    outb(GRAPHICS_CRTC_INDEX,0x0f);
    outb(GRAPHICS_CRTC_DATA,(unsigned char)(pos&0xff));
    outb(GRAPHICS_CRTC_INDEX,0x0e);
    outb(GRAPHICS_CRTC_DATA,(unsigned char)((pos>>8) & 0xff));
    vga_pos = pos;
    if (vga_pos > VGA_WIDTH * VGA_HEIGHT) {
        memcpy(VGA_MEM,VGA_MEM + VGA_WIDTH*2,VGA_WIDTH*(VGA_HEIGHT-1)*2);
        clear_down_row();
        set_cursor_pos(vga_pos - VGA_WIDTH - vga_pos % VGA_WIDTH);
    }
}

uint16_t coord_from_pos(uint16_t x, uint16_t y) {
    return (y*VGA_WIDTH)+x;
}

void noprint(const char *str) {
    char* cPtr = (char*)str;
    int i = 0;
    while (*cPtr != 0) {
        serial::write_serial(*cPtr);
        cPtr++;
    }
}

void print(const char *str) {
    char* cPtr = (char*)str;
    int i = 0;
    while (*cPtr != 0) {
        printc(*cPtr);
        cPtr++;
    }
    set_cursor_pos(vga_pos);
}

void gprintc(char c) {
    uint64_t index_font = (8*c);
    uint64_t x = (vga_pos%VGA_WIDTH)*8;
    uint64_t y = (vga_pos/VGA_WIDTH)*8;
    for (uint64_t i = 0; i < 8; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            if ((g_8x8_font[index_font]) & (0x80 >> j)) {
                write_pixel8(x+j,y,0x1f);//*(uint8_t*)((uint64_t)VGA_MEM+(y*320+(x+j))) = 0xff;
            } else {
                write_pixel8(x+j,y,1);
            }
        }
        index_font++;
        y += 1;
    }
}

void printc(char c) {
    switch (c) {
        case '\n':
            set_cursor_pos(vga_pos+VGA_WIDTH);
            break;
        case '\r':
            set_cursor_pos(vga_pos-(vga_pos%VGA_WIDTH));
            break;
        default:
            *(VGA_MEM+vga_pos*2) = c;
            if (g) { draw_font((vga_pos%VGA_WIDTH)*8,vga_pos/VGA_WIDTH*16,c,g_8x16_font,16,b); write_screen(); }
            vga_pos++;
            if (vga_pos >= VGA_WIDTH*VGA_HEIGHT) set_cursor_pos(vga_pos);
    }
#ifdef DEBUG_SERIAL
    serial::write_serial(c);
#endif
}

char intToStringOutput[128];
template <typename T>
const char* IntegerToString(T value) {
    uint8_t size = 0;
    uint64_t sizeTester = (uint64_t)value;
    while (sizeTester / 10 > 0) {
        sizeTester /= 10;
        size++;
    }
    uint8_t index = 0;
    uint64_t newValue = (uint64_t)value;
    while (newValue / 10 > 0) {
        uint8_t remainder = newValue % 10;
        newValue /= 10;
        intToStringOutput[size-index] = 48 + remainder;
        index++;
    }
    uint8_t remainder = newValue % 10;
    intToStringOutput[size-index] = 48 + remainder;
    intToStringOutput[size + 1] = 0;
    return intToStringOutput;
}

char floatToStringOutput[128];
const char* FloatToString(float value) {
    char* iPtr = (char*)IntegerToString((uint64_t)value);
    char* fPtr = floatToStringOutput;
    while (!iPtr != 0) {
        *fPtr = *iPtr;
        fPtr++;
        iPtr++;
    }
    return floatToStringOutput;
}

char hexToStringOutput[128];
template<typename T>
const char* HexToString(T value){
  T* valPtr = &value;
  uint8_t* ptr;
  uint8_t temp;
  uint8_t size = (sizeof(T)) * 2 - 1;
  uint8_t i;
  for (i = 0; i < size; i++){
    ptr = ((uint8_t*)valPtr + i);
    if (size >= (i*2+1)) {
        temp = ((*ptr & 0xF0) >> 4);
        hexToStringOutput[size - (i * 2 + 1)] = temp + (temp > 9 ? 55 : 48);
    }
    if (size >= (i*2)) {
        temp = ((*ptr & 0x0F));
        hexToStringOutput[size - (i * 2 + 0)] = temp + (temp > 9 ? 55 : 48);
    }
  }
  hexToStringOutput[size + 1] = 0;
  return hexToStringOutput;
}

int handle_format(bool print_characters, char* format, va_list ap) {
    if (*format == 0) return -1;
    switch (*format) {
        case 's':
            if (print_characters) print(va_arg(ap, char*));
            else noprint(va_arg(ap, char*));
            return 0;
        case '%':
            if (print_characters) printc('%');
            else serial::write_serial('%');
            return 0;
        case 'c':
            if (print_characters) printc((char)va_arg(ap, int));
            else serial::write_serial((char)va_arg(ap, int));
            return 0;
        case 'h':
            if (print_characters) print(HexToString(va_arg(ap, uint64_t)));
            else noprint(HexToString(va_arg(ap, uint64_t)));
            return 0;
        case 't':
            trace(32);
            return 0;
        case 'f':
            if (print_characters) print(FloatToString(va_arg(ap,double)));
            else noprint(FloatToString(va_arg(ap, double)));
            return 0;
        case 'x':
            if (print_characters) print(IntegerToString(va_arg(ap,uint64_t)));
            else noprint(IntegerToString(va_arg(ap, uint64_t)));
            return 0;
        default:
            if (print_characters) print("Invalid format");
            else noprint("Invalid format");
            return 0;
    }
}

void printf(const char* str, ...) {
    va_list ap;
    va_start(ap, str);
    char* cPtr = (char*)str;
    int i = 0;
    while (*cPtr != 0) {
        switch (*cPtr) {
            case '%':
                cPtr++;
                cPtr += handle_format(true,cPtr,ap);
                break;
            default:
                printc(*cPtr);
        }
        cPtr++;
    }
    set_cursor_pos(vga_pos);
    va_end(ap);
}

void noprintf(const char* str, ...) {
    va_list ap;
    va_start(ap, str);
    char* cPtr = (char*)str;
    int i = 0;
    while (*cPtr != 0) {
        switch (*cPtr) {
            case '%':
                cPtr++;
                cPtr += handle_format(false,cPtr,ap);
                break;
            default:
                serial::write_serial(*cPtr);
        }
        cPtr++;
    }
    set_cursor_pos(vga_pos);
    va_end(ap);
}

void debugf(const char *str, ...) {
#ifdef DEBUG
#ifndef DEBUG_SERIAL
    printf(str);
#else
    noprintf(str);
#endif
#endif
}

uint8_t attribute(uint8_t fc, uint8_t bc) {
    return fc | bc;
}

void clear(uint8_t color) {
    for (uint16_t i = 0; i < VGA_WIDTH*VGA_HEIGHT; i++) {
        *(VGA_MEM+i*2) = 0;
        *(VGA_MEM+i*2+1) = color;
    }
    set_cursor_pos(0);
}

void print_hex(uint8_t byte) {
    print(HexToString(byte));
}

void print_hex(uint16_t word) {
    print(HexToString(word));
}

void print_hex(uint32_t dword) {
    print(HexToString(dword));
}

void print_hex(uint64_t quad) {
    print(HexToString(quad));
}

void set_line_color(uint64_t line, uint8_t c, uint64_t start, uint64_t end) {
    for (uint16_t i = start; i < end; i++) {
        *(VGA_MEM+(i+(line*VGA_WIDTH))*2+1) = c;
    }
}

void enable_cursor(void) {
    outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 0x0D);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 0x0E);
}

void disable_cursor(void) {
    outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void clear_line(void) {
    uint8_t* line = (uint8_t*)(VGA_MEM+align_to_start(vga_pos,VGA_WIDTH)*2);
    for (uint64_t i = 0; i < VGA_WIDTH; i++) {
        *(line+i*2) = 0;
    }
    set_cursor_pos(align_to_start(vga_pos,VGA_WIDTH));
}

void new_line() {
    set_cursor_pos(coord_from_pos(0,vga_pos / VGA_WIDTH + 1));
}