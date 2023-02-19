#pragma once
#include <typedef.h>

#define MAX_FILES 1

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

    typedef struct ustar {
        file_entry_t files[MAX_FILES];
        ustar(uint8_t* buffer);
        void list_files();
    } ustar_t;
}