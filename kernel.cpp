#include <vga.h>
#include <idt.h>
#include <debug.h>
#include <memory.h>
#include <bitmap.h>
#include <asm.h>
#include <serial.h>
#include <disk.h>
#include <io.h>
#include <cfs.h>
#include <heap.h>
#include <font.h>
#include <acpi.h>
#include <timer.h>
#include <asm.h>
#include <pci.h>
#include <sound.h>

ptm_t* g_PTM = NULL;

bool test_heap() {
    void* a = heap::malloc(0x100);
    uint64_t aaddr = (uint64_t)a;
    void* b = heap::malloc(0x100);
    uint64_t baddr = (uint64_t)b;
    void* c = heap::malloc(0x100);
    void* d = heap::malloc(0x100);
    debugf("A: %h\n\rB: %h\n\rC: %h\n\rD: %h\n\r",a,b,c,d);
    heap::free(b);
    void* e = heap::malloc(0x100);
    uint64_t eaddr = (uint64_t)e;
    debugf("E: %h\n\r",e);
    heap::free(a);
    heap::free(e);
    void* f = heap::malloc(0x200);
    uint64_t faddr = (uint64_t)f;
    debugf("F: %h\n\r",f);
    heap::free(c);
    heap::free(d);
    heap::free(f);
    return (eaddr==baddr) && (aaddr==faddr);
}

ptm_t init_paging() {
    pt_t* pml4 = (pt_t*)request_page();
    memset(pml4, 0, 0x1000);
    ptm_t pm = ptm_t(pml4,get_memory_size()/0x1000);
    print("Created PTM and PML4\n\r");
    for (uint64_t i = 0; i < 0x800; i++) {
        pm.map((void*)(i*0x1000),(void*)(i*0x1000));
    }
    print("Identity mapped 512 pages at the start of memory\n\rLoading PML4...");
    asm("mov %0, %%cr3"::"r"(pml4));
    unlock_old_page_tables();
    print("done\n\r");
    mmap_entry_t* mmap = (mmap_entry_t*)MEMORY_MAP;
    for (uint64_t i = 0; i < memory_region_count; i++) {
        uint8_t* base = (uint8_t*)mmap[i].base;
        if (mmap[i].type != 1) {
            for (uint64_t j = 0; j < mmap[i].size; j+=0x1000) {
                pm.mark_page_used(base);
                pm.map(base,base);
                base += 0x1000;
            }
        }
    }
    print("Identity mapped everything\n\r");
    asm("mov %0, %%cr3"::"r"(pml4));
    return pm;
}

void init_disk() {
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

    uint16_t *buffer = (uint16_t*)heap::malloc(2*256);
    read_disk(&dev0,(uint8_t*)buffer,0,1);
    mbr_t* mbr = (mbr_t*)(&buffer[0xDB]);

    CFS::cfs_t cfs = CFS::cfs_t(mbr->partition0,&dev0, g_PTM);
    //cfs.list_files();
    serial::write_serial("Disk there is\n\r",15);
    //free(parse_font((uint8_t*)(cfs.read_file(1))));
    heap::free(buffer);
}

extern "C" void main() {
    set_cursor_pos(coord_from_pos(0,0));
    if (serial::init_serial()) {
        printf("Failed to init serial");
        return;
    }
    serial::write_serial("Just passing by.\n\r",18);
    init_vga();
    printf("Welcome to %s %s\n\r","longOS","dev snapshot");
    asm("cli");
    init_idt();
    PIT::init_timer();
    PCSPK::beep();
    
    convert_mmap_to_bmp();
    print_memory();

    g_PTM = &init_paging();
    ACPI::init_acpi();
    heap::init_heap((void*)0x0000100000000000,0x10);

    if (!test_heap()) {
        print("HEAP: Test failed!");
        asm("cli");
        for (;;);
    }
    
    ACPI::fadt_t* fadt = (ACPI::fadt_t*)ACPI::get_table("FACP");
    print("Preffered Power Management Mode: ");
    switch (fadt->preferred_power_management_profile) {
        case 0:
            print("Unspecified");
            break;
        default:
            print("Unknown");
    }
    print("\n\r");
    ACPI::detect_hardware();
    ACPI::enable_acpi();
    PCI::print_pci();
    while (1) {
        uint64_t time = PIT::millis_since_boot;
        printf("Time since boot: %x:%x:%x.%x           \r",((time / 1000)/60)/60,((time / 1000)/60)%60,(time / 1000)%60,time % 1000);
        PIT::sleep(10);
    }
    ACPI::shutdown();

    while (1) {
        asm("hlt");
    }
    return;
}