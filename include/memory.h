#pragma once
#include <typedef.h>
#include <bitmap.h>
extern uint8_t memory_region_count;

typedef struct mmap_entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t ext_attr;
} mmap_entry_t;

typedef struct pd_entry {
    bool present : 1;
    bool rw : 1;
    bool us : 1;
    bool wt : 1;
    bool cd : 1;
    bool accessed : 1;
    bool ignore0 : 1;
    bool lp : 1;
    bool ignore1 : 1;
    uint8_t avl : 3;
    uint64_t addr : 51;
    bool ne : 1;
} pd_entry_t;

typedef struct pt {
    pd_entry_t entries[512];
}__attribute__((aligned(0x1000))) pt_t;

typedef struct page_index {
    uint64_t pdp_i;
    uint64_t pd_i;
    uint64_t pt_i;
    uint64_t p_i;
    page_index(uint64_t vaddr);
} page_index_t;

typedef struct ptm {
    ptm(pt_t* pml4In, uint64_t page_count);
    pt_t* pml4;
    bitmap_t vmmap;
    uint64_t size;
    void map(void* vmem, void* pmem);
    void unmap(void* vmem);
    void* get_paddr(void* vaddr);
    void* allocate_page();
    void* allocate_pages(uint64_t);
    void mark_page_used(void*);
    void mark_page_unused(void*);
    void free_page(void*);
    void free_pages(void*,uint64_t);
} ptm_t;

void print_memory();
uint64_t get_memory_size();
uint64_t get_usable_memory_size();
void convert_mmap_to_bmp();
void reserve_page(void*);
void reserve_pages(void*,uint64_t);
void lock_page(void*);
void lock_pages(void*,uint64_t);
void unreserve_page(void*);
void unreserve_pages(void*,uint64_t);
void unlock_page(void*);
void unlock_pages(void*,uint64_t);
void memset(void*,uint8_t,uint64_t);
void memcpy(void*,void*,uint64_t);
void print_segments();
void* request_page();
void* request_pages(uint64_t);
uint64_t align(uint64_t value, uint64_t alignment);
uint64_t align_to_start(uint64_t value, uint64_t alignment);
bool is_in_entry(mmap_entry_t* entry, void* addr);
void unlock_old_page_tables();
void* convert_to_address(uint64_t pdp_i,uint64_t pd_i,uint64_t pt_i,uint64_t p_i);

extern bitmap_t memory_map;
extern uint32_t* _mmap;
extern uint8_t _start_all;
extern uint8_t _end_all;