#include <cfs.h>
#include <vga.h>

uint8_t fsbuffer[512];

cfs::cfs(partition_t partition,device_t* device,ptm_t* ptm) {
    dev = device;
    read_disk(dev,fsbuffer,partition.lba,1);
    memcpy(&header,fsbuffer,sizeof(header_t));
    start_lba = 2;
    uint8_t pages_for_root_dir = 1;//(header.root_dir_size * header.sectors_per_block * 512) / 0x1000;
    files = (file_entry*)ptm->allocate_pages(pages_for_root_dir);
    printf("%h\n\r",files);
    read_disk(dev,(uint8_t*)files,start_lba,8);//header.root_dir_size * header.sectors_per_block);
    memset(fsbuffer,0,9);
    memcpy(fsbuffer,header.vid,8);
    printf("CFS loaded\n\rVID: %s\n\rSPB: %x\n\rRDS: %x\n\rMagic: %h\n\r", fsbuffer,header.sectors_per_block,header.root_dir_size,*(uint32_t*)header.magic);
}

void cfs::list_files() {
    file_entry_t* file_list = files;
    file_list--;
    for (uint64_t i = 0; (i < ((header.root_dir_size * header.sectors_per_block * 512) / 16)) && i < 2; i++) {
        file_list++;
        if (file_list->flags&1!=0) {
            memset(fsbuffer,0,9);
            memcpy(fsbuffer,file_list->name,8);
            printf("Name: %s\n\r",fsbuffer);
            printf("File %x %x %h %h\n\r",i,file_list->flags & 1,file_list->lba,file_list->flags);
        }
        
    }
}