OUTPUT_FORMAT(binary)

ENTRY (_start)

INPUT (
    build/kernel/extboot.o
    build/kernel/kernel.o
    build/kernel/vga.o
    build/kernel/io.o
    build/kernel/idt.o
    build/kernel/asm.o
    build/kernel/debug.o
    build/kernel/memory.o
    build/kernel/bitmap.o
    build/kernel/task.o
    build/kernel/serial.o
    build/kernel/disk.o
    build/kernel/heap.o
    build/kernel/acpi.o
    build/kernel/timer.o
    build/kernel/pci.o
    build/kernel/sound.o
    build/kernel/gdt.o
    build/kernel/ustar.o
    build/kernel/vfs.o
    build/kernel/string.o
    build/kernel/convert.o
    build/kernel/cmos.o
    build/kernel/mouse.o
)

OUTPUT (
    build/kernel/kernel.bin
)

SECTIONS {
    /* Our origin is at 0x8000 */
    . = 0x8000;
    _start_all = .;

    .text : ALIGN(0x1000) {
        *(.text)
        
    }
    end_code = .;
    .data : ALIGN(0x1000) {
        *(.data)
    }
    .rodata : ALIGN(0x1000) {
        *(.rodata)
    }
    .bss : ALIGN(0x1000) {
        *(COMMON)
        *(.bss)
    }
    
    .stack : ALIGN(0x1000) {
        _stack_top = .;
    }
    _stack = _stack_top + 0x8000;
    . = _stack;
    
    .idt BLOCK(0x1000) : ALIGN(0x1000) {
        _idt = .;
    }
    . = _idt + 0x1000;
    
    .mmap : ALIGN(0x1000) {
        _mmap = .;
    }
    . = _mmap + 0x4000;
    
    _end_all = .;
}