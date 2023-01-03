#include <vga.h>
#include <idt.h>
#include <debug.h>
#include <memory.h>
#include <bitmap.h>
#include <asm.h>
#include <serial.h>

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

    printf("VRAM: %h\n\r%t", kpm.get_paddr((void*)0xb8000));

    for (;;);
    return;
}