#include <heap.h>
#include <memory.h>
#include <vga.h>

namespace heap {
    void* heap_start;
    void* heap_end;
    heap_seg_header_t* last_seg;
    
    void init_heap(void* addr, size_t pages) {
        void* pos = addr;
        for (size_t i = 0; i < pages; i++) {
            g_PTM->map(pos,request_page());
            g_PTM->mark_page_used(pos);
            pos = (void*)((size_t)pos+0x1000);
        }
        size_t heap_length = pages*0x1000;
        heap_start = addr;
        heap_end = (void*)((size_t)heap_start + heap_length);
        heap_seg_header_t* start_seg = (heap_seg_header_t*)addr;
        start_seg->size = heap_length - sizeof(heap_seg_header_t);
        start_seg->next = start_seg->prev = NULL;
        start_seg->free = true;
        last_seg = start_seg;
    }
    
    void free(void* addr) {
        heap_seg_header_t* seg = (heap_seg_header_t*)addr - 1;
        seg->free = true;
        seg->combine_forward();
        seg->combine_backward();
        debugf("Free %h\n\r",seg);
    }
    
    void* malloc(size_t size) {
        debugf("Attempt allocate %x bytes\n\r",size);
        if (size % 0x10 > 0) {
            size -= size%0x10;
            size += 0x10;
            debugf("Attempt allocate (realigned) %x bytes\n\r",size);
        }
        if (size == 0) return NULL;
        debugf("Start allocation of %x bytes\n\r",size);
        heap_seg_header_t* seg = (heap_seg_header_t*)heap_start;
        while (true) {
            if (seg->free) {
                debugf("Found free segment checking for %x bytes\n\r",size);
                if (seg->size > (size + 0x30)) {
                    seg->split(size);
                    seg->free = false;
                    debugf("Alloc %h %h\n\r",seg,size);
                    return (void*)((size_t)seg + sizeof(heap_seg_header_t));
                }
                if (seg->size >= size) {
                    seg->free = false;
                    debugf("Alloc %h %h\n\r",seg,seg->size);
                    return (void*)((size_t)seg + sizeof(heap_seg_header_t));
                }
            }
            if (seg->next == NULL) break;
            seg = seg->next;
        }
        expand_heap(size+sizeof(heap_seg_header_t));
        return malloc(size);
    }
    
    heap_seg_header_t* heap_seg_header_t::split(size_t size) {
        debugf("Attempt split to size %x bytes\n\r",size);
        if (size < 0x10) return NULL;
        size_t splitSize = this->size - size - sizeof(heap_seg_header_t);
        if (splitSize <  0x10) return NULL;
        heap_seg_header_t* new_seg = (heap_seg_header_t*)((size_t)this+size+sizeof(heap_seg_header_t));
        next->prev = new_seg;
        new_seg->next = next;
        next = new_seg;
        new_seg->prev = this;
        new_seg->free = free;
        new_seg->size = splitSize;
        this->size = size;
        if (last_seg == this) last_seg = new_seg;
        return new_seg;
    }
    
    void expand_heap(size_t size) {
        if (size % 0x1000 > 0) {
            size -= size % 0x1000;
            size += 0x1000;
        }
        debugf("Attempt extend by %x bytes\n\r",size);
        size_t pages = size/0x1000;
        heap_seg_header_t* new_seg = (heap_seg_header_t*)heap_end;
        for (size_t i = 0; i < pages; i++) {
            g_PTM->mark_page_used(heap_end);
            g_PTM->map(heap_end,request_page(false));
            heap_end = (void*)((size_t)heap_end+0x1000);
        }
        new_seg->free = true;
        new_seg->prev = last_seg;
        last_seg->next = new_seg;
        last_seg = new_seg;
        new_seg->next = NULL;
        new_seg->size = size-sizeof(heap_seg_header_t);
        new_seg->combine_backward();
    }
    
    void heap_seg_header_t::combine_forward() {
        if (next == NULL) return;
        if (!next->free) return;
        if (next == last_seg) last_seg = this;
        if (next->next != NULL) next->next->prev = this;
        size = size + next->size + sizeof(heap_seg_header_t);
    }
    
    void heap_seg_header_t::combine_backward() {
        if (prev != NULL && prev->free) prev->combine_forward();
    }
}