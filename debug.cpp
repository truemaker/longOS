#include <vga.h>
#include <asm.h>
#include <idt.h>
#include <io.h>
#include <memory.h>
#include <debug.h>

extern uint8_t end_extboot;

void trace(unsigned int frames){
    struct stackframe *stk;
    stk = (struct stackframe*) __builtin_frame_address(0);
    trace(frames,stk);
}

void trace(unsigned int frames, struct stackframe* stk) {
    print("Stack trace:\n\r");
    for(unsigned int frame = 0; stk && frame < frames; ++frame)
    {
        // Unwind to previous stack frame
        printf("    0x%h : 0x%h ", stk, stk->rip);
        if (stk->rip < (uint64_t)&_start_all || stk->rip > (uint64_t)&_end_all) {
          print("-> Interrupt (propably)");
        } else if (stk->rip < (uint64_t)&end_extboot ) {
          print("-> Bootloader");
        } else if (stk->rip > (uint64_t)&end_extboot && stk->rip < (uint64_t)&_end_all) {
          print("-> Kernel");
        }
        print("\n\r");
        stk = stk->ebp;
    }
}