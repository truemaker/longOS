[bits 16]
[org 0x600]

start:
cli                         ; We do not want to be interrupted
xor ax, ax                  ; 0 AX
mov ds, ax                  ; Set Data Segment to 0
mov es, ax                  ; Set Extra Segment to 0
mov ss, ax                  ; Set Stack Segment to 0
mov sp, 0x6000              ; Set Stack Pointer to 0x6000
.cpy:
    mov cx, 0x0100            ; 256 WORDs in MBR
    mov si, 0x7C00            ; Current MBR Address
    mov di, 0x0600            ; New MBR Address
    rep movsw                 ; Copy MBR
jmp 0:load              ; Jump to new Address

load:
    sti                         ; Start interrupts
    mov BYTE [bootDrive], dl    ; Save BootDrive
    .chk_partitions:           ; Check Partition Table For Bootable Partition
        mov bx, PT1               ; Base = Partition Table Entry 1
        mov cx, 4                 ; There are 4 Partition Table Entries
        .loop:
            mov al, BYTE [bx]       ; Get Boot indicator bit flag
            and al, 0x80           ; Check For Active Bit
            test al, al
            jnz .found          ; We Found an Active Partition
            add bx, 0x10            ; Partition Table Entry is 16 Bytes
            dec cx                  ; Decrement Counter
            jnz .loop           ; Loop
        jmp error                 ; error!
        .found:
            mov WORD [PToff], bx    ; Save Offset
            add bx, 8               ; Increment Base to LBA Address
    .rd_vbr:
        read_disk:
        mov cl, [bx]
        inc cl
        mov bx, 0x7c00
        mov ah, 0x02
        mov al, 1
        mov ch, 0x00
        mov dh, 0x00
        mov dl, [bootDrive]
        clc
        int 0x13
        jc error
    .run:
        cmp WORD [0x7DFE], 0xAA55 ; Check Boot Signature
        jne error                 ; Error if not Boot Signature
        mov si, WORD [PToff]      ; Set DS:SI to Partition Table Entry
        mov dl, BYTE [bootDrive]  ; Set DL to Drive Number
        jmp 0:0x7C00                ; Jump To VBR

error:
    mov bx, msg_err
    call print
    jmp $

print:
    mov ah, 0xe
.print_loop:
    mov al, [si]
    int 0x10
    inc si
    cmp byte [si], 0
    jnz .print_loop
    ret

times (218 - ($-$$)) nop      ; Pad for disk time stamp

DiskTimeStamp times 8 db 0    ; Disk Time Stamp
msg_err: db "Error",0
bootDrive db 0                ; Our Drive Number Variable
PToff dw 0                    ; Our Partition Table Entry Offset
times (0x1b4 - ($-$$)) nop    ; Pad For MBR Partition Table

UID times 10 db 0             ; Unique Disk ID
PT1:
    db 0x80
    times 3 db 0
    db 0x69
    times 3 db 0
    dd 0x1
    dd 2879

PT2 times 16 db 0             ; Second Partition Entry
PT3 times 16 db 0             ; Third Partition Entry
PT4 times 16 db 0             ; Fourth Partition Entry

times 510-($-$$) db 0
dw 0xAA55