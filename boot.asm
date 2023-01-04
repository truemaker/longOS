; 16 bit bootloader
[org 0x7c00]
[bits 16]

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
    mov bx, 0x8000
    mov ah, 0x02
    mov al, 64
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
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