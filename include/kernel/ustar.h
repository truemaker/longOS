#pragma once
#include <typedef.h>
#include <disk.h>
#include <vfs.h>

#define MAX_FILES 32

namespace USTAR {
    typedef struct file_entry {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        uint8_t typeflag;
        char linked[100];
        char signature[6];
        char version[2];
        char username[32];
        char groupname[32];
        uint64_t device_major_number;
        uint64_t device_minor_number;
        char filename_prefix[155];
    } file_entry_t;

    typedef struct file {
        file_entry_t entry;
        uint64_t lba;
    } file_t;

    typedef struct ustar {
        file_t files[MAX_FILES];
        ustar(uint8_t*);
        ustar(device_t*,uint64_t,uint64_t);
        void list_files(void);
        void mount(char*);
    } ustar_t;
}