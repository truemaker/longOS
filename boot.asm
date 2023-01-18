; 16 bit bootloader
[org 0x7c00]
[bits 16]
jmp short start
nop
fsh_vid: db "longOS  "
fsh_rds: db 1
fsh_spb: dw 8
fsh_total_blocks: dw 359
fsh_free_blocks: dw 350
fsh_magic: db 0x42, 0x69, 0x11, 0x11
fsh_reserved_sectors: db 1

start:
mov [BOOT_DISK], dl

xor ax, ax                          
mov es, ax
mov ds, ax

mov bp, 0x7c00
mov sp, bp

mov ah, 0x00
mov al, 0x2
int 0x10

mov si, msg_hello
call print

call read_root_dir
call read_disk

mov al, [BOOT_DISK]
push ax
push 0x8000
ret

jmp $

memcpy:
.cpy_loop:
    mov dx, [si]
    mov [di], dx
    inc si
    inc di
    dec cx
    cmp cx, 0
    jne .cpy_loop
    ret

print:
    mov ah, 0xe
.print_loop:
    mov al, [si]
    int 0x10
    inc si
    cmp byte [si], 0
    jnz .print_loop
    ret

read_disk:
    mov ax, 8
    mov cl, [kernel_pos+2]
    shl ecx, 8
    mov cl, [kernel_pos+1]
    shl ecx, 8
    mov cl, [kernel_pos]

    mul cx
    mov cx, ax
    add cl, [fsh_reserved_sectors]
    add cl, 2
    
    mov ax, 8
    mov bl, [kernel_size]
    mul bx

    mov bx, 0x8000
    mov ah, 0x02 
    mov ch, 0x00
    mov dh, 0x00
    mov dl, [BOOT_DISK]
    int 0x13
    jc disk_error
    ret

read_root_dir:
    mov bx, root_dir
    mov ah, 0x02 
    mov al, 8
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x3; Assuming we are in partition 1 (you should not do that)
    mov dl, [BOOT_DISK]
    int 0x13
    jc disk_error
    ret

disk_error:
    mov si, msg_disk_error
    call print
    jmp $

msg_disk_error: db "Disk Error!",0
msg_hello: db "Hello, World!",0xD,0xA,0
BOOT_DISK: db 0
buffer: db "BlubBlah!",0

times 510-($-$$) db 0
dw 0xaa55
root_dir:
kernel_name: db "OS   BIN" ; 8 bytes
db 0x89, 0 ; 2 bytes
kernel_pos: db 1,0,0 ; 3 byte
kernel_size: db 8 ; 1 byte
times 0x1200-($-$$) db 0