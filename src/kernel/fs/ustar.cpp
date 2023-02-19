#include <ustar.h>
#include <heap.h>
#include <memory.h>
#include <vga.h>

namespace USTAR {
    uint64_t oct2bin(uint8_t *str, int size) {
        uint64_t n = 0;
        uint8_t *c = str;
        while (size-- > 0) {
            n *= 8;
            n += *c - '0';
            c++;
        }
        return n;
    }

    ustar::ustar(uint8_t* buffer) {
        memset(this->files,0,sizeof(this->files));
        void* addr = (void*)buffer;
        for (uint64_t i = 0; i < MAX_FILES; i++) {
            file_entry_t* file = (file_entry_t*)addr;
            if (file->name == "\0") break;
            memcpy(this->files+i,file,512);
            uint64_t size = oct2bin((uint8_t*)file->size,11);
            addr += (((size + 511) / 512) + 1) * 512;
        }
    }

    void ustar::list_files(void) {
        for (uint64_t i = 0; i < MAX_FILES; i++) {
            file_entry_t* file = &this->files[i];
            if (file->name == "\0") break;
            print(file->name);
            print(" ");
            switch (file->typeflag) {
                case 0:
                case '0':print("file"); break;
                case '1':print("link"); break;
                case '2':print("symbolic"); break;
                case '3':print("chardev"); break;
                case '4':print("blockdev"); break;
                case '5':print("dir"); break;
                case '6':print("fifo"); break;
            }
            print(" ");
            print(file->username);
            print("\n\r");
        }
    }
}