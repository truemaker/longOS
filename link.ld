OUTPUT_FORMAT(binary)

ENTRY (_start)

INPUT (
    extboot.o
    kernel.o
    vga.o
    io.o
    idt.o
    asm.o
    debug.o
    memory.o
    bitmap.o
    task.o
    serial.o
    disk.o
    cfs.o
    heap.o
    font.o
    acpi.o
)

OUTPUT (
    kernel.bin
)

SECTIONS {
    /* Our origin is at 0x8000 */
    . = 0x8000;
    _start_all = .;

    .text : ALIGN(0x1000) {
        *(.text)
        
    }
    end_code = .;
    .idt BLOCK(0x1000) : ALIGN(0x1000) {
        _idt = .;
    }

    . = _idt + 0x1000; /* Move by 0x1000 to make space for the idt */

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
    .mmap : ALIGN(0x1000) {
        _mmap = .;
    }
    . = _mmap + 0x4000;
    _end_all = .;
}