#pragma once
#include <typedef.h>
#include <bitmap.h>
extern uint8_t memory_region_count;

#define MEMORY_MAP 0x7000
#define PAGE_SIZE ((uint64_t)0x1000)

typedef struct mmap_entry {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t ext_attr;
} mmap_entry_t;

typedef struct pd_entry {
    bool p : 1;
    bool rw : 1;
    bool us : 1;
    bool wt : 1;
    bool cd : 1;
    bool a : 1;
    bool ignore0 : 1;
    bool lp : 1;
    bool ignore1 : 1;
    bool used : 1;
    uint8_t avl : 2;
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
    uint64_t size;
    bool allocating;
    void map(void* vmem, void* pmem);
    void unmap(void* vmem);
    void* get_paddr(void* vaddr);
    bool get_present(void* vaddr);
    void* allocate_page(void);
    void* allocate_pages(uint64_t);
    void mark_page_used(void*);
    void mark_page_unused(void*);
    void free_page(void*);
    void free_pages(void*,uint64_t);
    void* find_free(uint64_t);
    bool is_used(void*);
    pd_entry_t* get_pdp(void*);
    pd_entry_t* get_pd(void*);
    pd_entry_t* get_pt(void*);
    pd_entry_t* get_page(void*);
} ptm_t;

void print_memory(void);
uint64_t get_memory_size(void);
uint64_t get_usable_memory_size(void);
void convert_mmap_to_bmp(void);
void reserve_page(void*);
void reserve_pages(void*,uint64_t);
void lock_page(void*);
void lock_pages(void*,uint64_t);
void unreserve_page(void*);
void unreserve_pages(void*,uint64_t);
void free_page(void*);
void free_pages(void*,uint64_t);
void memset(void*,uint8_t,uint64_t);
void memcpy(void*,void*,uint64_t);
bool memcmp(void*,void*,uint64_t);
void print_segments(void);
void* request_page(void);
void* request_pages(uint64_t);
uint64_t align(uint64_t value, uint64_t alignment);
uint64_t align_to_start(uint64_t value, uint64_t alignment);
bool is_in_entry(mmap_entry_t* entry, void* addr);
void unlock_old_page_tables(void);
void* index2address(uint64_t pdp_i,uint64_t pd_i,uint64_t pt_i,uint64_t p_i);
bool is_free(void*);

extern bitmap_t memory_map;
extern uint8_t* _mmap;
extern uint8_t _start_all;
extern uint8_t _end_all;
extern ptm_t *g_PTM;