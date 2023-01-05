#include <vga.h>
#include <idt.h>
#include <debug.h>
#include <memory.h>
#include <bitmap.h>
#include <asm.h>
#include <serial.h>
#include <disk.h>
#include <io.h>

uint16_t buffer[256];

ptm_t init_paging() {
    pt_t* pml4 = (pt_t*)request_page();
    memset(pml4, 0, 0x1000);
    ptm_t pm = ptm_t(pml4,get_memory_size()/0x1000);
    print("Created PTM and PML4\n\r");
    for (uint64_t i = 0; i < 0x200; i++) {
        pm.map((void*)(i*0x1000),(void*)(i*0x1000));
        pm.mark_page_used((void*)(i*0x1000));
    }
    print("Identity mapped 512 pages at the start of memory\n\rLoading PML4...");
    asm("mov %0, %%cr3"::"r"(pml4));
    unlock_old_page_tables();
    print("done\n\r");
    for (uint64_t i = 0x200; i < (get_memory_size()/0x1000); i++) {
        pm.map((void*)(i*0x1000),(void*)(i*0x1000));
    }
    print("Identity mapped everything\n\r");
    asm("mov %0, %%cr3"::"r"(pml4));
    return pm;
}

extern "C" void main() {
    set_cursor_pos(coord_from_pos(0,0));
    if (init_serial()) {
        printf("Failed to init serial");
        return;
    }
    write_serial("Just passing by.\n\r",18);
    printf("Welcome to %s %s\n\r","longOS","dev snapshot");
    asm("cli");
    init_idt();

    print_memory();
    convert_mmap_to_bmp();
    
    ptm_t kpm = init_paging();
    
    print_segments();
    clear();
    print("Init disk...\n\r");
    device_t dev0 = device_t(0x1f0,0x3F6,0xA0,"Disk 1");
    init_disk(&dev0);
    print_device(&dev0);

    device_t dev1 = device_t(0x1f0,0x3F6,0xB0,"Disk 2");
    init_disk(&dev1);
    print_device(&dev1);

    device_t dev2 = device_t(0x170,0x376,0xA0,"Disk 3");
    init_disk(&dev2);
    print_device(&dev2);

    device_t dev3 = device_t(0x170,0x376,0xB0,"Disk 4");
    init_disk(&dev3);
    print_device(&dev3);

    read_disk(&dev0,(uint8_t*)buffer,0,1);

    for (int i = 1; i <= 256; i++) {
        if ((((i - 1) % 16) != 0) && (i != 0)) print(" ");
        print_hex(buffer[i-1]);
        if (((i % 16) == 0) && (i != 0)) print("\n\r");
    }
    mbr_t* mbr = (mbr_t*)(&buffer[0xDB]);
    print_mbr(mbr);
    //printf("VRAM: %h\n\r%t", kpm.get_paddr((void*)0xb8000));

    for (;;);
    return;
}