#include <vga.h>
#include <asm.h>
#include <idt.h>
#include <io.h>
struct stackframe {
  struct stackframe* ebp;
  uint64_t rip;
};

void trace(unsigned int frames){
    struct stackframe *stk;
    stk = (struct stackframe*) __builtin_frame_address(0);
    print("Stack trace:\n\r");
    for(unsigned int frame = 0; stk && frame < frames; ++frame)
    {
        // Unwind to previous stack frame
        printf("    0x%h\n\r", stk->rip);
        stk = stk->ebp;
    }
}