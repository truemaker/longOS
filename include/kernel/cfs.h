#pragma once

#include <typedef.h>
#include <disk.h>
#include <memory.h>

namespace CFS {
    #define CFS_FILE_FLAG_PERMISSIONS 0xff00
    #define CFS_FILE_FLAG_ISFILE 0x0080
    #define CFS_FILE_FLAG_ISDIR 0x0040
    #define CFS_FILE_FLAG_ISLINK 0x0020
    #define CFS_FILE_FLAG_ISLABEL 0x0010
    #define CFS_FILE_FLAG_ISFIXED 0x0008
    #define CFS_FILE_FLAG_EXEC_LEVEL 0x0004
    #define CFS_FILE_FLAG_PRESENT 0x0001
    #define CFS_FILE_FLAG_TYPE 0x00f0
    
    typedef struct header {
        uint8_t jmp[3];
        char vid[8];
        uint8_t root_dir_size;
        uint16_t sectors_per_block;
        uint16_t total_blocks;
        uint16_t free_blocks;
        uint8_t magic[4];
        uint8_t reserved_sectors;
    } header_t;
    
    typedef struct file_entry {
        char name[8];
        uint16_t flags;
        uint8_t pos_lo;
        uint8_t pos_mid;
        uint8_t pos_hi;
        uint8_t size_lo;
        uint8_t size_mid;
        uint8_t size_hi;
    } file_entry_t __attribute__((packed));
    
    typedef struct cfs {
        header_t header;
        file_entry_t* files;
        uint32_t start_lba;
        device_t* dev;
        cfs(partition_t,device_t*,ptm_t*);
        void list_files();
        void print_file(file_entry_t* file);
        void recalculate_header();
        void* read_file(uint64_t,uint8_t*,uint64_t);
    } cfs_t;
}