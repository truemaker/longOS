.SILENT:
TARGET=x86_64
KERNEL_ARGS=-I"include/kernel" -ffreestanding -nostdlib -mno-red-zone -Wno-write-strings -fno-permissive -Wno-attributes -Wno-int-to-pointer-cast -Wno-pointer-arith
KERNEL_ARGS64=$(KERNEL_ARGS) -m64 -c
SRC=src
SRC_KERNEL=$(SRC)/kernel
SRC_BOOT=$(SRC)/boot
SRC_RES=$(SRC)/res
EMUARGS=-m 256M image.img -device qemu-xhci -audiodev alsa,id=speaker -soundhw all
GPP=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-g++
LD=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-ld
all:
	make run
	make cleanUp
build:
	dd if=/dev/zero of=zero.bin bs=512 count=26 status=none
	nasm -f bin $(SRC_BOOT)/boot.asm -o boot.bin
	nasm -f bin $(SRC_BOOT)/mbr.asm -o mbr.bin
	nasm -f bin $(SRC_RES)/font.asm -o font.bin
	nasm -i$(SRC_KERNEL) -f elf64 $(SRC_KERNEL)/extended_boot.asm -o extboot.o
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/kernel.cpp" -o "kernel.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/vga.cpp" -o "vga.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io.cpp" -o "io.o"
	$(GPP) $(KERNEL_ARGS64) -mgeneral-regs-only "$(SRC_KERNEL)/idt.cpp" -o "idt.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/asm.cpp" -o "asm.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/debug.cpp" -o "debug.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/memory.cpp" -o "memory.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/bitmap.cpp" -o "bitmap.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/task.cpp" -o "task.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/serial.cpp" -o "serial.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/disk.cpp" -o "disk.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/cfs.cpp" -o "cfs.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/heap.cpp" -o "heap.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/font.cpp" -o "font.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/acpi.cpp" -o "acpi.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/timer.cpp" -o "timer.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/pci.cpp" -o "pci.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/sound.cpp" -o "sound.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/gdt.cpp" -o "gdt.o"
	$(LD) -T "link.ld" -Map memory.map
	cat mbr.bin boot.bin kernel.bin > OS.bin
	dd if=/dev/zero of=image.img bs=512 count=2880 status=none
	dd if=OS.bin of=image.img conv=notrunc status=none
	dd if=font.bin of=image.img conv=notrunc seek=409600 status=none
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