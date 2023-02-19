#pragma once
#include <typedef.h>

namespace heap {
    typedef struct heap_seg_header {
        size_t size;
        heap_seg_header* next;
        heap_seg_header* prev;
        bool free;
        void combine_forward(void);
        void combine_backward(void);
        heap_seg_header* split(size_t size);
    } heap_seg_header_t;

    void init_heap(void*,size_t);
    void* malloc(size_t);
    void* calloc(size_t,uint8_t);
    void free(void*);
    void expand_heap(size_t);    
}
inline void* operator new(size_t s) {return heap::malloc(s);}
inline void* operator new[](size_t s) {return heap::malloc(s);}
inline void operator delete(void* ptr) {heap::free(ptr);}