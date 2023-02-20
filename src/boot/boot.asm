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

call read_tar
call parse_tar

mov si, msg_done
call print

mov al, [BOOT_DISK]
push ax
push 0x8000
ret
jmp $

convert_to_int:
    mov ax, 0
    mov bx, 8
.digit:
    mov dx, [si]
    cmp dx, ' '
    je .end
    mul bx
    sub dx, '0'
    add ax, dx
    inc si
    dec cx
    cmp cx, 0
    jg .digit
.end:
    ret

read_tar:
    mov bx, 0x6000
    mov dh, 0
    mov cl, 0x03
    mov al, 1
    mov ah, 0x02
    mov ch, 0
    mov dl, [BOOT_DISK]
    int 0x13
    jc disk_error
    ret

get_size:
    push si
    push cx
    push di
    push dx
    push bx
.size_bytes:
    push si
    mov si, msg_start_size
    call print
    pop si
    add si, 0x7c ; size
    mov cx, 11
    call convert_to_int
.size_sectors:
    mov dx, 0
    add ax, 511
    mov bx, 512
    div bx
    inc ax
.end:
    pop bx
    pop dx
    pop di
    pop cx
    pop si
    ret

parse_tar:
    mov si, 0x6000
    mov bx, 1
.look_file:
    push si
    mov di, buffer
    mov cx, 9
    call memcpy
    mov si, buffer
    mov di, kernel_name
    mov cx, 9
    call memcmp
    pop si
    cmp ax, 1
    je .found
    jmp $
.found:
    push bx
    call get_size
    pop bx
    mov bx, 1
    call read_kernel
.end:
    ret

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

memcmp:
    push bx
    push dx
.cmp_loop:
    mov dx, [si]
    mov bx, [di]
    cmp dx,bx
    jne .fail
    inc si
    inc di
    dec cx
    cmp cx, 0
    jne .cmp_loop
    pop dx
    pop bx
    mov ax, 1
    ret
.fail:
    pop dx
    pop bx
    mov ax, 0
    ret

print:
    pusha
    pushf
    mov ah, 0xe
.print_loop:
    mov al, [si]
    int 0x10
    inc si
    cmp byte [si], 0
    jnz .print_loop
    popf
    popa
    ret

read_kernel:
    mov cx, bx
    add cx, 0x03
    mov al, 124
    
    push es
    push 0x800
    pop es

    mov bx, 0
    mov dh, 0
    mov ah, 0x02
    mov ch, 0
    mov dl, [BOOT_DISK]
    int 0x13
    jc disk_error
    pop es
    ret

disk_error:
    mov si, msg_disk_error
    call print
    jmp $

msg_disk_error: db "Disk Error!",0
msg_hello: db "Hello, World!",0xD,0xA,0
msg_found: db "Found kernel.bin",0xD,0xA,0
msg_loading: db "Loading kernel.bin",0xD,0xA,0
msg_size: db "Got size",0xD,0xA,0
msg_start_size: db "Calculating size...",0xD,0xA,0
msg_done: db "Finished loading kernel.bin",0
BOOT_DISK: db 0
kernel_name: db "kernel.bin"
buffer: times 10 db 0

times 510-($-$$) db 0
dw 0xaa55