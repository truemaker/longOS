#include <io.h>
#include <vga.h>
#include <typedef.h>
#include <stdarg.h>
#include <debug.h>
#include <memory.h>

uint16_t vga_pos = 0;

void clear_down_row() {
    memset(VGA_MEM+VGA_WIDTH*(VGA_HEIGHT-1)*2,attribute(FG_WHITE,BG_BLUE),VGA_WIDTH*2);
    for (uint64_t i = 0; i < VGA_WIDTH; i++) {
        *(VGA_MEM+VGA_WIDTH*(VGA_HEIGHT-1)*2+i*2) = 0;
    }
}

void set_cursor_pos(uint16_t pos) {
    outb(0x3D4,0x0f);
    outb(0x3D5,(unsigned char)(pos&0xff));
    outb(0x3D4,0x0e);
    outb(0x3D5,(unsigned char)((pos>>8) & 0xff));
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

void print(const char *str) {
    char* cPtr = (char*)str;
    int i = 0;
    while (*cPtr != 0) {
        printc(*cPtr);
        cPtr++;
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
            set_cursor_pos(vga_pos+1);
    }
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

int handle_format(char* format, va_list ap) {
    if (*format == 0) return -1;
    switch (*format) {
        case 's':
            print(va_arg(ap, char*));
            return 0;
        case '%':
            printc('%');
            return 0;
        case 'c':
            printc((char)va_arg(ap, int));
            return 0;
        case 'h':
            print(HexToString(va_arg(ap, uint64_t)));
            return 0;
        case 't':
            trace(32);
            return 0;
        case 'f':
            print(FloatToString(va_arg(ap,float)));
            return 0;
        case 'x':
            print(IntegerToString(va_arg(ap,uint64_t)));
            return 0;
        default:
            print("Invalid format");
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
                cPtr += handle_format(cPtr,ap);
                break;
            default:
                printc(*cPtr);
        }
        cPtr++;
    }
    va_end(ap);
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

void print_hex(uint16_t word) {
    print(HexToString(word));
}