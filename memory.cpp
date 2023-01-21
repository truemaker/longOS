#include <memory.h>
#include <vga.h>
#include <bitmap.h>
#include <asm.h>
uint64_t free_mem;
uint64_t reserved_mem;
uint64_t used_mem;
bitmap_t memory_map;
void print_memory() {
    for (int i = 0; i < memory_region_count; i++) {
        mmap_entry_t* entry = (mmap_entry_t*)0x6000;
        entry += i;
        printf("Memory region at %h with size %h is %s\n\r", entry->base, entry->size, (entry->type == 1) ? "Usable" : ((entry->type == 3) ? "ACPI reclaimable" : ((entry->type == 4) ? "ACPI NVS Storage" : ((entry->type == 5) ? "broken" : "Reserved"))));
    }
}

uint64_t get_memory_size() {
    uint64_t size = 0;
    for (uint32_t i = 0; i < memory_region_count; i++) {
        mmap_entry_t* entry = (mmap_entry_t*)0x6000;
        entry += i;
        size += entry->size;
    }
    return size;
}

uint64_t get_usable_memory_size() {
    uint64_t size = 0;
    for (uint32_t i = 0; i < memory_region_count; i++) {
        mmap_entry_t* entry = (mmap_entry_t*)0x6000;
        entry += i;
        if (entry->type == 1) size += entry->size;
    }
    return size;
}

bool is_free(void* addr) {
    uint64_t index = (uint64_t)addr / 0x1000;
    return !memory_map[index];
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

void convert_mmap_to_bmp() {
    uint64_t pages = get_memory_size() / 4096;
    uint64_t bytes = pages / 32;
    printf("Memory bitmap is %x bytes and contains %x pages\n\r", bytes*4, pages);
    memory_map = bitmap_t();
    memory_map.size = pages+1;
    memory_map.bytes = _mmap;
    memset(_mmap,0,bytes*4);
    free_mem = get_memory_size();
    for (uint64_t i = 0; i < pages; i++) {
        bool free;
        for (uint64_t e = 0; e < memory_region_count; e++) {
            mmap_entry_t* entry = (mmap_entry_t*)0x6000;
            entry += e;
            if (is_in_entry(entry, (void*)(i*0x1000))) free = entry->type == 1;
        }
        if (!free) reserve_page((void*)(i*0x1000));
    }
    lock_pages((void*)0x2000,4); // Lock old PTs
    lock_pages((void*)&_start_all,((uint64_t)&_end_all - (uint64_t)&_start_all) / 4096); // Lock kernel
    lock_page((void*)0x6000); // Lock MMAP
}

void unlock_old_page_tables() {
    free_pages((void*)0x2000,4);
}

void reserve_page(void* addr) {
    if (memory_map[align_to_start((uint64_t)addr,0x1000)/0x1000]) return;
    memory_map.set(align_to_start((uint64_t)addr,0x1000)/0x1000,1);
    reserved_mem += 0x1000;
    free_mem -= 0x1000;
}

void unreserve_page(void* addr) {
    if (!memory_map[align_to_start((uint64_t)addr,0x1000)/0x1000]) return;
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
    if (memory_map[align_to_start((uint64_t)addr,0x1000)/0x1000]) return;
    memory_map.set(align_to_start((uint64_t)addr,0x1000)/0x1000,1);
    used_mem += 0x1000;
    free_mem -= 0x1000;
}

void free_page(void* addr) {
    if (!memory_map[align_to_start((uint64_t)addr,0x1000)/0x1000]) return;
    memory_map.set(align_to_start((uint64_t)addr,0x1000)/0x1000,0);
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

void memset(void* addr, uint8_t val, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        *(uint8_t*)((uint64_t)addr + i) = val;
    }
}

bool memcmp(void* a, void* b, uint64_t count) {
    uint8_t* ba = (uint8_t*)a;
    uint8_t* bb = (uint8_t*)b;
    for (uint64_t i = 0; i < count; i++) {
        if (*ba != *bb) return false;
        ba++;
        bb++;
    }
    return true;
}

void memcpy(void* dst, void* src, uint64_t count) {
    uint8_t* bdst = (uint8_t*)dst;
    uint8_t* bsrc = (uint8_t*)src;
    for (uint64_t i = 0; i < count; i++) {
        *bdst = *bsrc;
        bsrc++;
        bdst++;
    }
}

void print_segments() {
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
    printf("Used: %x %s\n\rFree: %x %s\n\rReserved: %x %s\n\r", displayu, unit[unitu], displayf, unit[unitf], displayr, unit[unitr]);
}

void* request_page(bool map_page=false) {
    debugf("Requested new page\n\r");
    for (uint64_t i = 0; i < memory_map.size; i++) {
        if (!memory_map[i]) {
            lock_page((void*)(i*0x1000));
            if (map_page) {
                if (!(g_PTM == ((void*)0))) {
                    void* addr = g_PTM->find_free(1);
                    g_PTM->mark_page_used(addr);
                    g_PTM->map(addr,(void*)(i*0x1000));
                    return addr;
                }
            }
            return (void*)(i*0x1000);
        }
    }
    print("Out of memory");
    for (;;);
    return nullptr; // Hope this never happens
}

void* request_pages(uint64_t count,bool map_page=true) {
    uint64_t move_by = 0;
    for (uint64_t i = 0; i < memory_map.size-count; i+=move_by) {
        bool usable = true;
        for (uint64_t j = 0; j < count; j++) {
            if (memory_map[i+j]) {
                move_by = j+1;
                usable = false;
                continue;
            }
        }
        if (usable) {
            lock_pages((void*)(i*0x1000),count);
            if (map_page) {
                if (!(g_PTM == ((void*)0))) {
                    if (!g_PTM->is_used((void*)(i*0x1000))) {
                        void* addr = (void*)(0x1000*i);
                        g_PTM->map(addr,addr);
                        return addr;
                    } else {
                        void* addr = g_PTM->find_free(count);
                        for (uint64_t j=0;j<count;j++) {
                            g_PTM->mark_page_used((void*)((uint64_t)addr + (j*0x1000)));
                            g_PTM->map((void*)((uint64_t)addr + (j*0x1000)),(void*)((i+j)*0x1000));
                        }
                        return addr;
                    }
                }
            }
            return (void*)(i*0x1000);
        }
    }
    print("Out of memory");
    for (;;);
    return nullptr; // Hope this never happens
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
    vmmap.size = page_count / 32 + 1;
    vmmap.bytes = (uint32_t*)request_pages(page_count / 32 / 0x1000 + 1);
    size = page_count;
}

void clear_table(pt_t* pt) {
    for (uint64_t i = 0; i < 512; i++) {
        *(uint64_t*)&pt->entries[i] = 0;
    }
}

void ptm_t::map(void* vmem, void* pmem) {
    debugf("Mapping %h -> %h\n\r",vmem,pmem);
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

    pde = pt->entries[pi.p_i];
    pde.addr = (uint64_t)pmem >> 12;
    pde.p = true;
    pde.rw = true;
    pt->entries[pi.p_i] = pde;
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
    uint64_t pageIndex = align_to_start((uint64_t)page,0x1000);
    if (vmmap[pageIndex]) return;
    vmmap.set(pageIndex,1);
}

void ptm_t::mark_page_unused(void* page) {
    uint64_t pageIndex = align_to_start((uint64_t)page,0x1000)/0x1000;
    if (!vmmap[pageIndex]) return;
    vmmap.set(pageIndex,0);
}

bool ptm_t::is_used(void* page) {
    uint64_t index = (uint64_t)page/0x1000;
    if (index > size) return get_paddr(page) != (void*)0;
    return vmmap[index];
}

void* ptm_t::allocate_page() {
    allocating = true;
    for (uint64_t i = 0; i<size; i++) {
        if (is_used((void*)(i*0x1000))) continue;
        mark_page_used((void*)(i*0x1000));
        map((void*)(i*0x1000),request_page());
        debugf("PALLOC %h\n\r",i*0x1000);
        allocating = false;
        return (void*)(i*0x1000);
    }
    print("Out of virtual memory");
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
    print("Out of virtual memory");
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