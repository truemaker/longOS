TARGET=x86_64
ARGS=-Iinclude -ffreestanding -nostdlib -mno-red-zone -Wno-write-strings -fpermissive
ARGS64=$(ARGS) -m64 -c
EMUARGS=-m 256M image.img -device qemu-xhci -soundhw all -audiodev alsa,id=speaker
GPP=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-g++
LD=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-ld
all:
	make run
	make cleanUp
build:
	dd if=/dev/zero of=zero.bin bs=512 count=26
	nasm -f bin boot.asm -o boot.bin
	nasm -f bin mbr.asm -o mbr.bin
	nasm -f bin font.asm -o font.bin
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
	$(GPP) $(ARGS64) "font.cpp" -o "font.o"
	$(GPP) $(ARGS64) "acpi.cpp" -o "acpi.o"
	$(GPP) $(ARGS64) "timer.cpp" -o "timer.o"
	$(GPP) $(ARGS64) "pci.cpp" -o "pci.o"
	$(GPP) $(ARGS64) "sound.cpp" -o "sound.o"
	$(LD) -T "link.ld" -Map memory.map
	cat mbr.bin boot.bin kernel.bin > OS.bin
	dd if=/dev/zero of=image.img bs=512 count=2880
	dd if=OS.bin of=image.img conv=notrunc
	dd if=font.bin of=image.img conv=notrunc seek=409600
run: build
	qemu-system-$(TARGET) $(EMUARGS) -serial stdio
debug: build
	qemu-system-$(TARGET) $(EMUARGS) -serial file:log.log -no-reboot
	make clean

kvm: build
	kvm-spice $(EMUARGS) -serial stdio
	make clean

clean:
	rm -rf *.o *.bin *.img

cleanUp: clean
	rm -rf *.log