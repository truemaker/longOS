#include <ustar.h>
#include <heap.h>
#include <memory.h>
#include <vga.h>
#include <vfs.h>
#include <string.h>
#include <errorcodes.h>

namespace USTAR {
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

    ustar::ustar(device_t* dev, uint64_t start, uint64_t length) {
        memset(this->files,0,sizeof(this->files));
        if (!length) return;
        uint64_t current_sector = start;
        void* buf = (void*)heap::malloc(512);
        for (uint64_t i = 0; i < MAX_FILES; i++) {
            if (current_sector >= start+length) break;
            read_disk(dev,(uint8_t*)buf,current_sector,1);
            file_entry_t* file = (file_entry_t*)buf;
            if (file->name[0] == '\0') break;
            memcpy(this->files+i,file,512);
            this->files[i].lba = current_sector;
            uint64_t size = oct2bin((uint8_t*)file->size,11);
            current_sector += (((size + 511) / 512) + 1);
        }
        delete buf;
    }

    void ustar::list_files(void) {
        for (uint64_t i = 0; i < MAX_FILES; i++) {
            file_entry_t* file = &this->files[i].entry;
            if (file->name[0] == '\0') break;
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

    void ustar::mount(char* path) {
        printf("[USTAR] Mounting ustar to %s\n\r",path);
        uint32_t err = VFS::vfs_mount(path,this,0x01);
        if (err) { printf("[USTAR] Failed to mount to %s due to ",path);
            switch (err) {
                case EINVAL: print("EINVAL"); break;
                case ERELATIVE: print("ERELATIVE"); break;
                case EUSED: print("EUSED"); break;
            }
        }
        print("\n\r");
    }
}