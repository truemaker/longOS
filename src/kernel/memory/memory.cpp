#include <memory.h>
#include <vga.h>
#include <bitmap.h>
#include <asm.h>
uint64_t free_mem = 0;
uint64_t reserved_mem = 0;
uint64_t used_mem = 0;
uint64_t total_memory = 0;
uint64_t usable_memory = 0;
bool scanned_total = false;
bool scanned_usable = false;
bitmap_t memory_map;
void print_memory(void) {
    for (uint64_t i = 0; i < memory_region_count; i++) {
        mmap_entry_t* entry = (mmap_entry_t*)MEMORY_MAP;
        entry += i;
        printf("[MEMORY] Region at %h with size %h is %s\n\r", entry->base, entry->size, (entry->type == 1) ? "Usable" : ((entry->type == 3) ? "ACPI reclaimable" : ((entry->type == 4) ? "ACPI NVS Storage" : ((entry->type == 5) ? "broken" : "Reserved"))));
    }
}

uint64_t get_memory_size(void) {
    if (scanned_total) return total_memory;
    uint64_t size = 0;
    mmap_entry_t* entry = (mmap_entry_t*)MEMORY_MAP;
    for (uint64_t i = 0; i < memory_region_count; i++) {
        if (entry->base > size) size = entry->base;
        size += entry->size;
        entry++;
    }
    total_memory = size;
    scanned_total = true;
    return total_memory;
}

uint64_t get_usable_memory_size(void) {
    if (scanned_usable) return usable_memory;
    uint64_t size = 0;
    for (uint32_t i = 0; i < memory_region_count; i++) {
        mmap_entry_t* entry = (mmap_entry_t*)MEMORY_MAP;
        entry += i;
        if (entry->type == 1) size += entry->size;
    }
    usable_memory = size;
    scanned_usable = true;
    return usable_memory;
}

bool is_free(void* addr) {
    uint64_t index = (uint64_t)addr / 0x1000;
    return !memory_map.get(index);
}

uint64_t align_to_start(uint64_t value, uint64_t alignment) {
    return value - (value % alignment);
}

uint64_t align(uint64_t value, uint64_t alignment) {
    return value + (alignment - (value % alignment));
}

bool is_in_entry(mmap_entry_t* entry, void* addr) {
    return ((entry->size + entry->base) > ((uint64_t) addr)) && (((uint64_t) addr) > entry->base);
}

void convert_mmap_to_bmp(void) {
    uint64_t pages = (get_memory_size() & ~0xfff) >> 12;
    uint64_t bytes = pages >> 3;
    printf("[MEMORY] Bitmap is %x bytes and contains %x pages\n\r", bytes, pages);
    memory_map = bitmap_t();
    memory_map.size = (pages+1) / 8;
    memory_map.bytes = _mmap;
    memset(_mmap,0,0x4000);
    free_mem = get_memory_size();
    used_mem = 0;
    reserved_mem = 0;
    for (uint64_t i = 0; i < pages; i++) {
        bool free;
        for (uint64_t e = 0; e < memory_region_count; e++) {
            mmap_entry_t* entry = (mmap_entry_t*)MEMORY_MAP;
            entry += e;
            if (is_in_entry(entry, (void*)(i*0x1000))) free = entry->type == 1;
        }
        if (!free) reserve_page((void*)(i*0x1000));
    }
    lock_pages((void*)&_end_all,7); // Lock old PTs
    lock_pages((void*)&_start_all,((uint64_t)&_end_all - (uint64_t)&_start_all) / 4096); // Lock kernel
    lock_page((void*)MEMORY_MAP); // Lock MMAP
}

void unlock_old_page_tables(void) {
    free_pages((void*)&_end_all,7);
}

void reserve_page(void* addr) {
    if (memory_map.get(align_to_start((uint64_t)addr,0x1000)/0x1000)) return;
    memory_map.set(align_to_start((uint64_t)addr,0x1000)/0x1000,1);
    reserved_mem += 0x1000;
    free_mem -= 0x1000;
}

void unreserve_page(void* addr) {
    if (!memory_map.get(align_to_start((uint64_t)addr,0x1000)/0x1000)) return;
    memory_map.set(align_to_start((uint64_t)addr,0x1000)/0x1000,0);
    reserved_mem -= 0x1000;
    free_mem += 0x1000;
}

void reserve_pages(void* addr, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        reserve_page((void*)((uint64_t)addr+i*0x1000));
    }
}

void unreserve_pages(void* addr, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        unreserve_page((void*)((uint64_t)addr+i*0x1000));
    }
}
void lock_page(void* addr) {
    uint64_t index = align_to_start((uint64_t)addr,0x1000)/0x1000;
    if ((uint64_t)addr >= get_memory_size()) {printf("[MEMORY] Outside of physical memory: lock\n\r%t");asm("cli");for(;;);}
    if (memory_map.get(index)) return;
    memory_map.set(index,1);
    used_mem += 0x1000;
    free_mem -= 0x1000;
}

void free_page(void* addr) {
    uint64_t index = align_to_start((uint64_t)addr,0x1000)/0x1000;
    if ((uint64_t)addr >= get_memory_size()) {printf("[MEMORY] Outside of physical memory: free\n\r%t");asm("cli");for(;;);}
    if (memory_map.get(index)) return;
    memory_map.set(index,0);
    used_mem -= 0x1000;
    free_mem += 0x1000;
}

void lock_pages(void* addr, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        lock_page((void*)((uint64_t)addr+i*0x1000));
    }
}

void free_pages(void* addr, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        free_page((void*)((uint64_t)addr+i*0x1000));
    }
}

void memset(void* addr, uint8_t sval, uint64_t count) {
    if(!count){return;} // nothing to set?
    uint64_t val = (sval & 0xFF); // create a 64-bit version of 'sval'
    val |= ((val << 8) & 0xFF00);
    val |= ((val << 16) & 0xFFFF0000);
    val |= ((val << 32) & 0xFFFFFFFF00000000);
    
    while(count >= 8){ *(uint64_t*)addr = (uint64_t)val; addr += 8; count -= 8; }
    while(count >= 4){ *(uint32_t*)addr = (uint32_t)val; addr += 4; count -= 4; }
    while(count >= 2){ *(uint16_t*)addr = (uint16_t)val; addr += 2; count -= 2; }
    while(count >= 1){ *(uint8_t*)addr = (uint8_t)val; addr += 1; count -= 1; }
    return; 
}

bool memcmp(void* a, void* b, uint64_t count) {
    if(!count){return true;} // nothing to compare?
    while(count >= 8){ if (*(uint64_t*)a != *(uint64_t*)b) return false; a += 8; b += 8; count -= 8; }
    while(count >= 4){ if (*(uint32_t*)a != *(uint32_t*)b) return false; a += 4; b += 4; count -= 4; }
    while(count >= 2){ if (*(uint16_t*)a != *(uint16_t*)b) return false; a += 2; b += 2; count -= 2; }
    while(count >= 1){ if (*(uint8_t*)a != *(uint8_t*)b) return false; a += 1; b += 1; count -= 1; }
    return true;
}

void memcpy(void* dst, void* src, uint64_t count) {
    if(!count){return;} // nothing to copy?
    while(count >= 8){ *(uint64_t*)dst = *(uint64_t*)src; dst += 8; src += 8; count -= 8; }
    while(count >= 4){ *(uint32_t*)dst = *(uint32_t*)src; dst += 4; src += 4; count -= 4; }
    while(count >= 2){ *(uint16_t*)dst = *(uint16_t*)src; dst += 2; src += 2; count -= 2; }
    while(count >= 1){ *(uint8_t*)dst = *(uint8_t*)src; dst += 1; src += 1; count -= 1; }
    return;
}

void print_segments(void) {
    const char* unit[] = {"Bytes", "KB", "MB", "GB"};
    uint8_t unitu = 0;
    uint8_t unitf = 0;
    uint8_t unitr = 0;
    uint64_t displayu = used_mem;
    uint64_t displayf = free_mem;
    uint64_t displayr = reserved_mem;
    while (displayu > 10240 && unitu < 4) {
        displayu /= 1024;
        unitu++;
    }
    while (displayf > 10240 && unitf < 4) {
        displayf /= 1024;
        unitf++;
    }
    while (displayr > 10240 && unitr < 4) {
        displayr /= 1024;
        unitr++;
    }
    printf("[MEMORY] Used: %x %s\n\r[MEMORY] Free: %x %s\n\r[MEMORY] Reserved: %x %s\n\r", displayu, unit[unitu], displayf, unit[unitf], displayr, unit[unitr]);
}

uint64_t page_bitmap_index = 0;
void* request_page() {
    for (; page_bitmap_index < memory_map.size * 8; page_bitmap_index++){
        if (memory_map[page_bitmap_index] == true) continue;
        lock_page((void*)(page_bitmap_index * 4096));
        return (void*)(page_bitmap_index * 4096);
    }
    return NULL; // Page Frame Swap to file
}

void* request_pages(uint64_t count) {
    debugf("Requested new pages\n\r");
    uint64_t i = 0;
    uint64_t pages = 0;
    while (i<(get_memory_size()/0x1000)) {
        debugf("Checking %h\n\r",i*0x1000);
        while (i<(get_memory_size()/0x1000)) {
            if (!memory_map.get(i)) break;
            debugf("Checked %h x\n\r",i*0x1000);
            pages = 0;
            i++;
            debugf("Checking %h\n\r",i*0x1000);
        }
        if (!(i < (get_memory_size()/0x1000))) { print("[MEMORY] Out of memory: Multiple"); for (;;); return 0; }
        debugf("Checked %h v %x\n\r",i*0x1000,pages+1);
        pages++;
        if (pages > (count-1)) break;
        i++;
    }
    if (pages < count) { print("[MEMORY] Out of memory: Multiple page"); for (;;); return 0; }
    uint64_t addr = 0x1000 * i;
    if (addr > get_memory_size()) { print("[MEMORY] Something went wrong while allocating"); for(;;); return 0; }
    lock_pages((void*)addr - (0x1000*count),count);
    debugf("Found pages %h - %h\n\r", addr - (0x1000*count), addr);
    return (void*)addr;
}

page_index::page_index(uint64_t addr) {
    uint64_t vaddr = addr;
    vaddr >>= 12;
    p_i = vaddr & 0x1ff;
    vaddr >>= 9;
    pt_i = vaddr & 0x1ff;
    vaddr >>= 9;
    pd_i = vaddr & 0x1ff;
    vaddr >>= 9;
    pdp_i = vaddr & 0x1ff;
}

ptm_t::ptm(pt_t* pml4In, uint64_t page_count) {
    pml4 = pml4In;
    vmmap = bitmap_t();
    vmmap.size = (page_count + 1)/8;
    vmmap.bytes = (uint8_t*)request_pages((page_count / 0x1000 / 8 + 1));
    memset(vmmap.bytes,0,(page_count / 0x1000 / 8 + 1)*0x1000);
    size = page_count;
}

void clear_table(pt_t* pt) {
    for (uint64_t i = 0; i < 512; i++) {
        *(uint64_t*)&pt->entries[i] = 0;
    }
}

void ptm_t::map(void* vmem, void* pmem) {
    vmem = (void*)(((uint64_t)vmem >> 12) << 12);
    vmem = (void*)(((uint64_t)vmem >> 12) << 12);
    pmem = (void*)(((uint64_t)pmem >> 12) << 12);
    pmem = (void*)(((uint64_t)pmem >> 12) << 12);
    debugf("Mapping %h -> %h\n\rPML4: %h\n\r",vmem,pmem,pml4);
    page_index_t pi = page_index_t((uint64_t)vmem);
    pd_entry_t pde;
    pde = pml4->entries[pi.pdp_i];
    pt_t* pdp;
    if (!pde.p) {
        debugf("New PDP\n\r");
        pdp = (pt_t*)request_page();
        clear_table(pdp);
        pde.addr = (uint64_t)pdp >> 12;
        pde.p = true;
        pde.rw = true;
        pml4->entries[pi.pdp_i] = pde;
    } else {
        pdp = (pt_t*)((uint64_t)pde.addr << 12);
    }
    debugf("PDP: %h\n\r",pdp);
    pde = pdp->entries[pi.pd_i];
    pt_t* pd;
    if (!pde.p) {
        debugf("New PD\n\r");
        pd = (pt_t*)request_page();
        clear_table(pd);
        pde.addr = (uint64_t)pd >> 12;
        pde.p = true;
        pde.rw = true;
        pdp->entries[pi.pd_i] = pde;
    } else {
        pd = (pt_t*)((uint64_t)pde.addr << 12);
    }
    debugf("PD: %h\n\r",pd);
    pde = pd->entries[pi.pt_i];
    pt_t* pt;
    if (!pde.p) {
        debugf("New PT\n\r");
        pt = (pt_t*)request_page();
        clear_table(pt);
        pde.addr = (uint64_t)pt >> 12;
        pde.p = true;
        pde.rw = true;
        pd->entries[pi.pt_i] = pde;
    } else {
        pt = (pt_t*)((uint64_t)pde.addr << 12);
    }
    debugf("PT: %h\n\r",pt);
    pde = pt->entries[pi.p_i];
    pde.addr = (uint64_t)pmem >> 12;
    pde.rw = true;
    pde.p = true;
    pt->entries[pi.p_i] = pde;
    read_cr3();
    debugf("Mapping complete\n\r");
}

void ptm_t::unmap(void* vmem) {
    page_index_t pi = page_index_t((uint64_t)vmem);
    pd_entry_t pde;
    pde = pml4->entries[pi.pdp_i];
    pt_t* pdp;
    if (!pde.p) {
        return; // If the address isn't mapped we don't need to unmap it
    } else {
        pdp = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pdp->entries[pi.pd_i];
    pt_t* pd;
    if (!pde.p) {
        return;
    } else {
        pd = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pd->entries[pi.pt_i];
    pt_t* pt;
    if (!pde.p) {
        return;
    } else {
        pt = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pt->entries[pi.p_i];
    memset(&pde,0,sizeof(pde));
    reload_cr3();
    debugf("Unmap %h\n\r",vmem);
}

bool ptm_t::get_present(void* vaddr) {
    uint64_t offset = ((uint64_t)vaddr) % 0x1000;
    page_index_t pi = page_index_t(align_to_start((uint64_t)vaddr,0x1000));
    pd_entry_t pde;
    pde = pml4->entries[pi.pdp_i];
    pt_t* pdp;
    if (!pde.p) {
        return 0;
    } else {
        pdp = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pdp->entries[pi.pd_i];
    pt_t* pd;
    if (!pde.p) {
        return 0;
    } else {
        pd = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pd->entries[pi.pt_i];
    pt_t* pt;
    if (!pde.p) {
        return 0;
    } else {
        pt = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pt->entries[pi.p_i];
    return pde.p;
}

void* ptm_t::get_paddr(void* vaddr) {
    uint64_t offset = ((uint64_t)vaddr) % 0x1000;
    page_index_t pi = page_index_t(align_to_start((uint64_t)vaddr,0x1000));
    pd_entry_t pde;
    pde = pml4->entries[pi.pdp_i];
    pt_t* pdp;
    if (!pde.p) {
        return (void*)0;
    } else {
        pdp = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pdp->entries[pi.pd_i];
    pt_t* pd;
    if (!pde.p) {
        return (void*)0;
    } else {
        pd = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pd->entries[pi.pt_i];
    pt_t* pt;
    if (!pde.p) {
        return (void*)0;
    } else {
        pt = (pt_t*)((uint64_t)pde.addr << 12);
    }

    pde = pt->entries[pi.p_i];
    return (void*)((pde.addr << 12) + offset);
}

void ptm_t::mark_page_used(void* page) {
    uint64_t pageIndex = align_to_start((uint64_t)page,0x1000)/0x1000;
    if (pageIndex >= vmmap.size) return;
    if (vmmap[pageIndex]) return;
    vmmap.set(pageIndex,1);
}

void ptm_t::mark_page_unused(void* page) {
    uint64_t pageIndex = align_to_start((uint64_t)page,0x1000)/0x1000;
    if (pageIndex >= vmmap.size) return;
    if (!vmmap[pageIndex]) return;
    vmmap.set(pageIndex,0);
}

bool ptm_t::is_used(void* page) {
    uint64_t index = (uint64_t)page/0x1000;
    if (index >= size) return get_present(page);
    return vmmap.get(index);
}

void* ptm_t::allocate_page(void) {
    allocating = true;
    for (uint64_t i = 0; i<size; i++) {
        if (is_used((void*)(i*0x1000))) continue;
        mark_page_used((void*)(i*0x1000));
        map((void*)(i*0x1000),request_page());
        debugf("PALLOC %h\n\r",i*0x1000);
        allocating = false;
        return (void*)(i*0x1000);
    }
    print("[MEMORY] Out of virtual memory");
    for (;;);
    return (void*)0;
}

void ptm_t::free_page(void* page) {
    void* aligned_page = (void*)align_to_start((uint64_t)page,0x1000);
    unmap(get_paddr(aligned_page));
    mark_page_unused(aligned_page);
}

void* ptm_t::allocate_pages(uint64_t count) {
    debugf("Allocating %x pages\n\r",count);
    uint64_t move_by = 0;
    for (uint64_t i = 0; i < vmmap.size-count; i+=move_by) {
        bool usable = true;
        for (uint64_t j = 0; j < count; j++) {
            if (is_used((void*)((i+j)*0x1000))) {
                move_by = j+1;
                usable = false;
            }
        }
        if (usable) {
            for (uint64_t j = 0; j < count; j++) {
                mark_page_used((void*)((i+j)*0x1000));
                map((void*)((i+j)*0x1000),request_page());
            }
            return (void*)(i*0x1000);
        }
    }
    print("[MEMORY] Out of virtual memory");
    for (;;);
    return nullptr; // Hope this never happens
}

void* ptm_t::find_free(uint64_t count) {
    uint64_t move_by = 0;
    for (uint64_t i = 0; i < vmmap.size-count; i+=move_by) {
        bool usable = true;
        for (uint64_t j = 0; j < count; j++) {
            if ((void*)((i+j)*0x1000)) {
                move_by = j+1;
                usable = false;
            }
        }
        if (usable) {
            for (uint64_t j = 0; j < count; j++) {
                mark_page_used((void*)((i+j)*0x1000));
            }
            return (void*)(i*0x1000);
        }
    }
    return nullptr; // Hope this never happens
}

void ptm_t::free_pages(void* page, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        free_page((void*)((uint64_t)page + 0x1000 * i));
    }
}

void* index2address(uint64_t pdp_i,uint64_t pd_i,uint64_t pt_i,uint64_t p_i) {
    uint64_t vaddr = pdp_i;
    vaddr <<= 9;
    vaddr = vaddr | pd_i;
    vaddr <<= 9;
    vaddr = vaddr | pt_i;
    vaddr <<= 9;
    vaddr = vaddr | p_i;
    vaddr <<= 12;
    return (void*)vaddr;
}