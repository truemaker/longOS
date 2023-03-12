.SILENT:
TARGET=x86_64
KERNEL_ARGS=-I"include/kernel" -ffreestanding -nostdlib -mno-red-zone -Wno-write-strings -fno-permissive -Wno-attributes -Wno-int-to-pointer-cast -Wno-pointer-arith -O0
KERNEL_ARGS64=$(KERNEL_ARGS) -m64 -c
SRC=src
SRC_KERNEL=$(SRC)/kernel
SRC_BOOT=$(SRC)/boot
SRC_RES=$(SRC)/res
BUILD=build
BUILD_KERNEL=$(BUILD)/kernel
BUILD_BOOT=$(BUILD)/boot
BUILD_RES=$(BUILD)/res
EMUARGS=-m 256M image.img -device qemu-xhci -audiodev alsa,id=speaker -soundhw all
GPP=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-g++
LD=/usr/local/$(TARGET)elfgcc/bin/$(TARGET)-elf-ld
all:
	make run
	make cleanUp
prepare:
	mkdir -p build/kernel build/boot build/res
boot: prepare
	nasm -f bin $(SRC_BOOT)/boot.asm -o $(BUILD_BOOT)/boot.bin
	nasm -f bin $(SRC_BOOT)/mbr.asm -o $(BUILD_BOOT)/mbr.bin
kernel:
	echo "Building entry"
	nasm -i$(SRC_KERNEL) -f elf64 $(SRC_KERNEL)/extended_boot.asm -o $(BUILD_KERNEL)/extboot.o
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/kernel.cpp" -o "$(BUILD_KERNEL)/kernel.o"
	echo "Building vga"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/vga/vga.cpp" -o "$(BUILD_KERNEL)/vga.o"
	echo "Building io"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io/debug.cpp" -o "$(BUILD_KERNEL)/debug.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io/serial.cpp" -o "$(BUILD_KERNEL)/serial.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io/disk.cpp" -o "$(BUILD_KERNEL)/disk.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io/pci.cpp" -o "$(BUILD_KERNEL)/pci.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io/sound.cpp" -o "$(BUILD_KERNEL)/sound.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/io/cmos.cpp" -o "$(BUILD_KERNEL)/cmos.o"
	echo "Building util"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/util/string.cpp" -o "$(BUILD_KERNEL)/string.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/util/convert.cpp" -o "$(BUILD_KERNEL)/convert.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/util/asm.cpp" -o "$(BUILD_KERNEL)/asm.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/util/bitmap.cpp" -o "$(BUILD_KERNEL)/bitmap.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/util/io.cpp" -o "$(BUILD_KERNEL)/io.o"
	echo "Building memory"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/memory/memory.cpp" -o "$(BUILD_KERNEL)/memory.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/memory/heap.cpp" -o "$(BUILD_KERNEL)/heap.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/memory/gdt.cpp" -o "$(BUILD_KERNEL)/gdt.o"
	echo "Building fs"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/fs/ustar.cpp" -o "$(BUILD_KERNEL)/ustar.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/fs/vfs.cpp" -o "$(BUILD_KERNEL)/vfs.o"
	echo "Building power"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/power/acpi.cpp" -o "$(BUILD_KERNEL)/acpi.o"
	echo "Building interrupt"
	$(GPP) $(KERNEL_ARGS64) -mgeneral-regs-only "$(SRC_KERNEL)/interrupt/idt.cpp" -o "$(BUILD_KERNEL)/idt.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/interrupt/timer.cpp" -o "$(BUILD_KERNEL)/timer.o"
	echo "Building others"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/task.cpp" -o "$(BUILD_KERNEL)/task.o"
	$(GPP) $(KERNEL_ARGS64) "$(SRC_KERNEL)/font.cpp" -o "$(BUILD_KERNEL)/font.o"
build: prepare
#	nasm -f bin $(SRC_RES)/font.asm -o $(BUILD_RES)/font.bin
	make boot
	make kernel
	$(LD) -T "link.ld" -Map memory.map
	tar --create -H ustar --file $(BUILD)/kernel.tar -C $(BUILD_KERNEL) kernel.bin
	cat $(BUILD_BOOT)/mbr.bin $(BUILD_BOOT)/boot.bin $(BUILD)/kernel.tar > $(BUILD)/OS.bin
	dd if=/dev/zero of=image.img bs=512 count=2880 status=none
	dd if=$(BUILD)/OS.bin of=image.img conv=notrunc status=none
#	dd if=$(BUILD_RES)/font.bin of=image.img conv=notrunc seek=409600 status=none
run: build
	qemu-system-$(TARGET) $(EMUARGS) -serial stdio
debug: build
	qemu-system-$(TARGET) $(EMUARGS) -serial file:log.log -no-reboot
	make clean

kvm: build
	kvm-spice $(EMUARGS) -serial stdio
	make clean

clean: prepare
	rm -rf *.o *.bin *.img
	rm -rf build

cleanUp: clean
	rm -rf *.log