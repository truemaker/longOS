#include <cfs.h>
#include <vga.h>

uint8_t fsbuffer[512];

cfs::cfs(partition_t partition,device_t* device,ptm_t* ptm) {
    dev = device;
    read_disk(dev,fsbuffer,partition.lba,1);
    memcpy(&header,fsbuffer,sizeof(header_t));
    start_lba = partition.lba + header.reserved_sectors;
    uint8_t pages_for_root_dir = 1;//((header.root_dir_size * header.sectors_per_block * 512) & ~0xfff) >> 12;
    files = (file_entry*)ptm->allocate_pages(pages_for_root_dir);
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
    memset(buffer,0,10);
    char* tmp0 = name;
    char* tmp1 = buffer;
    int moved = 0;
    for (int i = 0; i < 5 && *tmp0 != ' '; i++, tmp0++, tmp1++) {
        *tmp1 = *tmp0;
        moved++;
    }
    *tmp1 = '.';
    memcpy((void*)((uint64_t)buffer + moved + 1),(void*)((uint64_t)name + 8-3),3);
    return moved + 4;
}

void cfs::print_file(file_entry* file) {
    uint8_t len = fill_file_name(file->name, fsbuffer);
    printf("    %s ",fsbuffer);
    for (uint8_t i = 0; i < (8-len); i++) {
        print(" ");
    }
    switch (file->flags & CFS_FILE_FLAG_TYPE) {
        case CFS_FILE_FLAG_ISFILE:
            print("FILE   ");
            break;
        case CFS_FILE_FLAG_ISLINK:
            print("LINK   ");
            break;
        case CFS_FILE_FLAG_ISDIR:
            print("DIR   ");
            break;
        default:
            print("INVALID");
    }
    print(" ");
    const char* units[] = {"Bytes", "KB", "MB", "GB"};
    uint8_t unit = 0;
    uint64_t display = file->size * header.sectors_per_block * 512;
    while (display > 10240 && unit < 4) {
        display /= 1024;
        unit++;
    }
    printf("%x %s %h", display, units[unit], file->lba);
    print("\n\r");
}

void cfs::list_files() {
    fill_volume_name(header.vid,fsbuffer);
    printf("Volume %s contains:\n\r",fsbuffer);
    file_entry_t* file_list = files;
    for (uint64_t i = 0; (i < ((header.root_dir_size * header.sectors_per_block * 512) / 16)); i++) {
        if (file_list->flags&CFS_FILE_FLAG_PRESENT!=0) {
            print_file(file_list);
            //printf("%h\n\r",file_list->flags);
        }
        file_list = (file_entry_t*)((uint64_t)file_list + 16);
    }
    const char* units[] = {"Bytes", "KB", "MB", "GB"};
    uint8_t unitf = 0;
    uint8_t unitt = 0;
    uint64_t displayf = header.free_blocks * header.sectors_per_block * 512;
    uint64_t displayt = header.total_blocks * header.sectors_per_block * 512;
    while (displayf > 10240 && unitf < 4) {
        displayf /= 1024;
        unitf++;
    }
    while (displayt > 10240 && unitt < 4) {
        displayt /= 1024;
        unitt++;
    }
    printf("%x %s of %x %s are free\n\r",displayf,units[unitf],displayt,units[unitt]);
}

void cfs::recalculate_header() {
    uint16_t blocks = header.total_blocks;
    uint64_t used = 0;
    printf("%x ",blocks);
    file_entry_t* file_list = files;
    for (uint64_t i = 0; (i < ((header.root_dir_size * header.sectors_per_block * 512) / 16)); i++) {
        if (file_list->flags&CFS_FILE_FLAG_PRESENT!=0) {
            printf("%x ",used);
            used += file_list->size;
            printf("%x ",used);
            printf("%x ",file_list->size);
        }
        file_list = (file_entry_t*)((uint64_t)file_list + 16);
    }
    printf("%x ",blocks);
    header.free_blocks = blocks - used;
    printf("%x",header.free_blocks);
}