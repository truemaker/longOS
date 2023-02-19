#include <io.h>
#include <serial.h>
namespace serial {
#define COM1 0x3f8
   bool init_serial(void) {
      outb(COM1 + 1, 0x00);
      outb(COM1 + 3, 0x80);
      outb(COM1 + 0, 0x03);
      outb(COM1 + 1, 0x00);
      outb(COM1 + 3, 0x03);
      outb(COM1 + 2, 0xC7);
      outb(COM1 + 4, 0x0B);
      outb(COM1 + 4, 0x1E);
      outb(COM1 + 0, 0xAE);
      if(inb(COM1 + 0) != 0xAE) {
         return 1;
      }
      outb(COM1 + 4, 0x0F);
      return 0;
   }
   
   bool serial_received(void) {
      return inb(COM1 + 5) & 1;
   }
   
   char read_serial(void) {
      while (!serial_received());
      return inb(COM1);
   }
   
   bool is_transmit_empty(void) {
      return inb(COM1 + 5) & 0x20;
   }
   
   void write_serial(char a) {
      while (!is_transmit_empty());
      outb(COM1,a);
   }
   
   void write_serial(char* a, uint32_t len) {
       char* str = (char*)a;
       for (uint32_t i = 0; i < len; i++) {
           write_serial(*str);
           str++;
       }
   }
}