TARGET=x86_64
ARGS=-Iinclude -ffreestanding -nostdlib -mno-red-zone -Wno-write-strings -fpermissive
ARGS64=$(ARGS) -m64 -c
GPP=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-g++
LD=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-ld
all:
	make run
	make clean
build:
	dd if=/dev/zero of=zero.bin bs=512 count=26
	nasm -f bin boot.asm -o boot.bin
	nasm -f bin mbr.asm -o mbr.bin
	nasm -f elf64 extended_boot.asm -o extboot.o
	$(GPP) $(ARGS64) "kernel.cpp" -o "kernel.o"
	$(GPP) $(ARGS64) "vga.cpp" -o "vga.o"
	$(GPP) $(ARGS64) "io.cpp" -o "io.o"
	$(GPP) $(ARGS64) -mgeneral-regs-only "idt.cpp" -o "idt.o"
	$(GPP) $(ARGS64) "asm.cpp" -o "asm.o"
	$(GPP) $(ARGS64) "debug.cpp" -o "debug.o"
	$(GPP) $(ARGS64) "memory.cpp" -o "memory.o"
	$(GPP) $(ARGS64) "bitmap.cpp" -o "bitmap.o"
	$(GPP) $(ARGS64) "task.cpp" -o "task.o"
	$(GPP) $(ARGS64) "serial.cpp" -o "serial.o"
	$(GPP) $(ARGS64) "disk.cpp" -o "disk.o"
	$(GPP) $(ARGS64) "cfs.cpp" -o "cfs.o"
	$(GPP) $(ARGS64) "heap.cpp" -o "heap.o"
	$(LD) -T "link.ld" -Map memory.map
	cat mbr.bin boot.bin kernel.bin > OS.bin
	dd if=/dev/zero of=image.img bs=512 count=2880
	dd if=OS.bin of=image.img conv=notrunc
run: build
	qemu-system-$(TARGET) -m 256M image.img -serial stdio
#kvm-spice -m 256M OS.bin

clean:
	rm -rf *.o *.bin *.img