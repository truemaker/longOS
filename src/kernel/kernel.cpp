#include <vga.h>
#include <idt.h>
#include <debug.h>
#include <memory.h>
#include <bitmap.h>
#include <asm.h>
#include <serial.h>
#include <disk.h>
#include <io.h>
#include <heap.h>
#include <font.h>
#include <acpi.h>
#include <timer.h>
#include <asm.h>
#include <pci.h>
#include <sound.h>
#include <defines.h>
#include <gdt.h>
#include <task.h>
#include <ustar.h>
#include <typedef.h>
#include <vfs.h>
#include <string.h>
#include <cmos.h>

ptm_t* g_PTM = NULL;
extern uint64_t _stack;
extern uint8_t kernel_sectors;
uint16_t buffer0[256];
uint16_t buffer1[2048];

bool test_heap(void) {
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

ptm_t init_paging(void) {
    pt_t* pml4 = (pt_t*)request_page();
    memset(pml4, 0, 0x1000);
    ptm_t pm = ptm_t(pml4,0xffffffffffffffff/0x1000);
    g_PTM = &pm;
    print("[KERNEL] Created PTM and PML4\n\r");
    for (uint64_t i = 0; i < 0x200; i++) {
        pm.map((void*)(i*0x1000),(void*)(i*0x1000));
    }
    print("[KERNEL] Identity mapped 512 pages at the start of memory\n\r[KERNEL] Loading PML4...");
    asm("mov %0, %%cr3"::"r"(pml4));
    unlock_old_page_tables();
    print("done\n\r");
    mmap_entry_t* mmap = (mmap_entry_t*)MEMORY_MAP;
    for (uint64_t i = 0; i < memory_region_count; i++) {
        uint8_t* base = (uint8_t*)mmap[i].base;
        if (mmap[i].type != 1) {
            for (uint64_t j = 0; j < mmap[i].size; j+=0x1000) {
                pm.map(base,base);
                pm.mark_page_used(base);
                base += 0x1000;
            }
        }
    }
    print("[KERNEL] Identity mapped everything\n\r");
    asm("mov %0, %%cr3"::"r"(pml4));
    return pm;
}

char* cd_names[] {
    "/dev/cda",
    "/dev/cdb",
    "/dev/cdc",
    "/dev/cdd"
};

char* hd_names[] {
    "/dev/hda",
    "/dev/hdb",
    "/dev/hdc",
    "/dev/hdd"
};

device_t* init_disk(void) {
    print("[KERNEL] Init disk...\n\r");

    device_t devs[4];

    devs[0] = device_t(0x1f0,0x1f6,0xA0,"Disk 1");
    devs[1] = device_t(0x1f0,0x1f6,0xB0,"Disk 2");
    devs[2] = device_t(0x170,0x176,0xA0,"Disk 3");
    devs[3] = device_t(0x170,0x176,0xB0,"Disk 4");

    bool found = false;
    uint64_t hd = 0;
    uint64_t cd = 0;
    uint64_t select = 0;
    for (uint64_t i = 0; i < 4; i++) {
        init_disk(&devs[i]);
        if (devs[i].type == DEV_PATAPI || devs[i].type == DEV_SATAPI) {
            printf("[KERNEL] Found %sPI drive\n\r",devs[i].type == DEV_PATAPI ? "PATA" : "SATA");
            VFS::add_device(devdup(&devs[i]),cd_names[cd]);
            cd++;
        } else if (devs[i].type == DEV_PATA || devs[i].type == DEV_SATA) {
            if (!found) { found = true; select = i; }
            printf("[KERNEL] Found %s drive\n\r",devs[i].type == DEV_PATA ? "PATA" : "SATA");
            VFS::add_device(devdup(&devs[i]),hd_names[hd]);
            hd++;
        }
    }

    if (found) return devdup(&devs[select]);

    print("[DISK] No connected drives are of type PATA or SATA\n\r");
    asm("cli");
    for (;;);
}

namespace VGASELECT {
    void move(uint8_t);
    void init();
    uint64_t selection;
    uint64_t follow;
    bool g,ok,retry,can_c;
    char* mode_names[] = { "80x25 text", "90x30 text", "90x60 text" };
    uint64_t mode_widths[] = { 80, 90, 90 };
    uint64_t mode_heights[] = { 25, 30, 60 };
    void put_symbol(uint64_t x, uint64_t y, uint8_t c) {
        set_cursor_pos(coord_from_pos(x,y));
        printc(c);
    }

    void put_string(uint64_t x, uint64_t y, char* s) {
        set_cursor_pos(coord_from_pos(x,y));
        print(s);
    }

    void the_secret() {
        for (uint64_t i = 0; i < 25; i++) {
            if ((i / 5)==0 || (i / 5)==4) set_line_color(i,BG_WHITE | FG_LIGHTBLUE, 0, 80);
            if ((i / 5)==1 || (i / 5)==3) set_line_color(i,BG_WHITE | FG_LIGHTRED, 0, 80);
            if ((i / 5)==2)               set_line_color(i,BG_WHITE | FG_WHITE, 0, 80);
            for (uint64_t j = 0; j < 80; j++) {
                put_symbol(j,i,0xb1);
            }
        }
    }

    void init() {
        set_mode(80,25);
        clear();
        selection = 0;
        can_c = true;
        g = false;
        retry = false;
        ok = false;
        follow = 0;
        for (uint64_t i = 0; i < 25; i++) {
            put_symbol(0,i,0xba);
            put_symbol(79,i,0xba);
        }
        for (uint64_t i = 0; i < 80; i++) {
            put_symbol(i,0,0xcd);
            put_symbol(i,24,0xcd);
            put_symbol(i,2,0xcd);
        }
        put_string(1,1,"longOS VGA Configuration");
        set_line_color(1,BG_BLUE | FG_YELLOW,1,25);
        put_symbol(0,0,0xc9);
        put_symbol(0,24,0xc8);
        put_symbol(79,0,0xbb);
        put_symbol(79,24,0xbc);
        put_symbol(0,2,0xcc);
        put_symbol(79,2,0xb9);
        for (uint64_t i = 3; i < 24; i++) {
            put_symbol(39,i,0xba);
        }
        put_symbol(39,2,0xcb);
        put_symbol(39,24,0xca);
        put_string(40,3,"Right Text");
        set_line_color(3,BG_BLUE | FG_YELLOW,40,50);
        for (uint64_t i = 0; i < 3; i++) {
            set_cursor_pos(coord_from_pos(1,i+3));
            print(mode_names[i]);
        }
        disable_cursor();
    }
    
    void secret_kh(uint8_t scancode) {
        switch (scancode) {
            case 0x81: init(); main_keyboard_handler = move; break;
            default: break;
        }
    }

    void move(uint8_t scancode) {
        switch (scancode) {
	        case 0x50: selection++; break;
	        case 0x48: selection--; break;
            case 0x9c: if (ok) { retry = true; } ok = true; break;
            case 0x14: if (follow) { follow = 1; g = false; } else { follow++; } break;
            case 0x13: if (follow != 1) { follow = 0; g = false; } else { follow++; } break;
            case 0x1e: if (follow != 2) { follow = 0; g = false; } else { follow++; } break;
            case 0x31: if (follow != 3) { follow = 0; g = false; } else { follow++; } break;
            case 0x1f: if (follow != 4) { follow = 0; g = false; } else { follow++; } break;
            case 0x22: g = true; follow = 0; break;
	        default: break;
	    }
        if ((long long)selection < 0) selection = 2;
        selection = selection % 3;
    }

    void run() {
        main_keyboard_handler = move;
        while (1) {
            init();
            uint64_t sel = selection;
            while (1) {
                if (g && follow == 5) { can_c = false; print_trans(); main_keyboard_handler = secret_kh; while (!can_c) asm("hlt"); }
                if (follow == 5) { can_c = false; the_secret(); main_keyboard_handler = secret_kh; while (!can_c) asm("hlt"); }
                sel = selection;
                for (uint64_t i = 0; i < 3; i++) {
                    set_line_color(i+3,(sel == i) ? 0xf1 : 0x1f,1,11);
                }
                if (ok) break;
            }
            set_mode(mode_widths[sel],mode_heights[sel]);
#ifndef VGASELECT_INSTANT_SELECT
            clear();
            
#ifdef VGASELECT_TIMEOUT_REVOKE
            printf("Selected: %s\n\rPress Enter to revoke within the next %x second(s).",mode_names[sel],VGASELECT_TIMEOUT_REVOKE/1000);
            PIT::sleep(VGASELECT_TIMEOUT_REVOKE);
#else
            printf("Selected: %s\n\rPress Enter to revoke within 3 seconds.",mode_names[sel]);
            PIT::sleep(3000);
#endif
            if (retry) continue;
#endif
            main_keyboard_handler = 0;
            clear();
            enable_cursor();
            return;
        }
    }
}

void taskA() {
    print("Hello from process a\n\r");
    yield();
}

void taskB() {
    print("Hello from process b\n\r");
    yield();
}

void desktop() {
    gclear(FG_DARKGRAY);
    write_screen();
    for (;;);
}

extern "C" void main() {
    init_vga();
    init_gdt();
    asm("cli");
    init_idt();
    PIT::init_timer();

    device_t boot_disk = device_t(0x1f0,0x3f6,0xA0,"");
    init_disk(&boot_disk);
    uint8_t* rest_of_kernel = (uint8_t*)0x18000;
    read_disk(&boot_disk,rest_of_kernel,131,(uint64_t)kernel_sectors);

#ifdef RUN_VGASELECT
    VGASELECT::run();
#endif

#ifndef MINIMAL_MODE
    if (serial::init_serial()) {
        printf("[SERIAL] Failed to init serial");
        return;
    }
    serial::write_serial("Just passing by.\n\r",18);
#endif
    printf("Welcome to %s %s\n\r","longOS","dev snapshot");
    PCSPK::beep();
    
    convert_mmap_to_bmp();
    print_memory();

    init_paging();
#ifndef MINIMAL_MODE
    ACPI::init_acpi();
    init_task();
#endif
    heap::init_heap((void*)0x0000100000000000,0x10);

    if (!test_heap()) {
        print("[HEAP] Test failed!");
        asm("cli");
        for (;;);
    }
    
#ifndef MINIMAL_MODE
    VFS::install_vfs();
    device_t* disk = init_disk();

    ACPI::enable_acpi();
    //ACPI::detect_hardware();
    //PCI::iterate_pci();
#endif
    print("[KERNEL] init done\n\r");
#ifndef MINIMAL_MODE
    fork((void*)taskA);
    fork((void*)taskB);
    yield();
    print("[KERNEL] Hello from kernel\n\r");
    print_segments();
    g_PTM->print_used();
#endif
    PIT::sleep(1000);
    enter_graphics();
    gclear(FG_DARKGRAY);
    write_screen();
    fork((void*)desktop);
    yield();
    for (;;);
    //while (1) {
    //    uint64_t time = PIT::millis_since_boot;
    //    print("\r");
    //    //clear_line();
    //    set_cursor_pos(0);
    //    CMOS::RTC::print_time();
    //    //printf("Time since boot: %x:%x:%x.%x",((time / 1000)/60)/60,((time / 1000)/60)%60,(time / 1000)%60,time % 1000);
    //    PIT::sleep(10);
    //}
#ifndef MINIMAL_MODE
    ACPI::shutdown();
#endif

    for (;;) asm("hlt");
    return;
}