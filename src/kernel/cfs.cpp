#include <cfs.h>
#include <vga.h>
#include <heap.h>

namespace CFS {
    uint8_t fsbuffer[512];

    cfs::cfs(partition_t partition,device_t* device,ptm_t* ptm) {
        dev = device;
        read_disk(dev,fsbuffer,partition.lba,1);
        memcpy(&header,fsbuffer,sizeof(header_t));
        start_lba = partition.lba + header.reserved_sectors;
        uint8_t pages_for_root_dir = align(((header.root_dir_size * header.sectors_per_block * 512) & ~0xfff),0x1000) >> 12;
        files = (file_entry*)request_pages(pages_for_root_dir);
        g_PTM->mark_page_used(files);
        g_PTM->map(files,files);
        read_disk(dev,(uint8_t*)files,start_lba,header.root_dir_size*header.sectors_per_block);
        read_disk(dev,fsbuffer,partition.lba,1);
        recalculate_header();
        memcpy(fsbuffer,&header,sizeof(header_t));
        write_disk(dev,fsbuffer,partition.lba,1);
        print("CFS loaded\n\r");
    }

    void fill_volume_name(char* name, char* buffer) {
        memset(buffer,0,9);
        char* tmp0 = name;
        char* tmp1 = buffer;
        for (int i = 0; i < 8 && *tmp0 != ' '; i++, tmp0++, tmp1++) {
            *tmp1 = *tmp0;
        }
    }

    uint8_t fill_file_name(char* name, char* buffer) {
    }

    void cfs::print_file(file_entry* file) {
    }

    void cfs::list_files() {
    }

    void cfs::recalculate_header() {
    }

    void* cfs::read_file(uint64_t id, uint8_t* buffer,uint64_t block) {
    }
}