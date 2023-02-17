[bits 16]
; extended boot aka stage 2
[global _start]
_start:
pop ax
mov [bdisk], al
mov [bpart], ah

mov si, msg_hello
call print

call detect_memory

call enableA20

cli
lgdt [g_desc]

mov eax, cr0
or eax, 1
mov cr0, eax

jmp codeseg:start_pm

jmp $

vesa_error:
    mov si, msg_no_vesa
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

enableA20:
    call A20check
a20_bios_func:
    mov     ax,2403h                ;--- A20-Gate Support ---
    int     15h
    jb      A20ns                  ;INT 15h is not supported
    cmp     ah,0
    jnz     A20ns                  ;INT 15h is not supported
    
    mov     ax,2402h                ;--- A20-Gate Status ---
    int     15h
    jb      A20failed              ;couldn't get status
    cmp     ah,0
    jnz     A20failed              ;couldn't get status
    
    cmp     al,1
    jz      A20done                 ;A20 is already activated
    
    mov     ax,2401h                ;--- A20-Gate Activate ---
    int     15h
    jb      A20failed              ;couldn't activate the gate
    cmp     ah,0
    jnz     A20failed              ;couldn't activate the gate
 
a20_activated:                  ;go on
    call A20check
a20_kbc:
    cli
 
    call    a20wait
    mov     al,0xAD
    out     0x64,al
 
    call    a20wait
    mov     al,0xD0
    out     0x64,al
 
    call    a20wait2
    in      al,0x60
    push    eax
 
    call    a20wait
    mov     al,0xD1
    out     0x64,al
 
    call    a20wait
    pop     eax
    or      al,2
    out     0x60,al
 
    call    a20wait
    mov     al,0xAE
    out     0x64,al
 
    call    a20wait
    sti
    jmp after_kbc
 
a20wait:
        in      al,0x64
        test    al,2
        jnz     a20wait
        ret
 
 
a20wait2:
        in      al,0x64
        test    al,1
        jz      a20wait2
        ret
after_kbc:
    call A20check
fastA20:
    in al, 0x92
    test al, 2
    jnz after
    or al, 2
    and al, 0xFE
    out 0x92, al
after:
    mov bx, 0xffff
chk_loop:
    call A20check
    sub bx, 1
    jnz chk_loop
    jmp A20failed
A20check:
    call check_a20
    test ax,ax
    jz .end  
.succes:
    call A20done
    pop ax
.end:
    ret
A20done:
    mov si, msg_a20_on
    call print
    ret
A20failed:
    mov si, msg_a20_off
    call print
    jmp $
A20ns:
    mov si, msg_a20_ns
    call print
    jmp $

detect_memory:
    mov ax, 0
    mov es, ax
    mov di, 0x7000
    mov edx, 0x534d4150
    xor ebx, ebx
.repeat:
    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    cmp ebx, 0
    je .finish
    add di, 24
    inc byte [memory_region_count]
    jmp .repeat
.finish:
    ret

check_a20:
test_a20:
    pushf
    push ds
    push es
    push di
    push si
    cli
    xor ax, ax ; ax = 0
    mov es, ax
    not ax ; ax = 0xFFFF
    mov ds, ax
    mov di, 0x0500
    mov si, 0x0510
    mov al, byte [es:di]
    push ax
    mov al, byte [ds:si]
    push ax
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
    cmp byte [es:di], 0xFF
    pop ax
    mov byte [ds:si], al
    pop ax
    mov byte [es:di], al
    mov ax, 0
    je check_a20__exit
    mov ax, 1
check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf
    ret

msg_hello: db "Hello and welcome in the extended boot!", 0xD, 0xA, 0
msg_no_vesa: db "No VESA", 0
msg_a20_off: db "Couldn't activate a20", 0
msg_a20_on: db "Activated a20", 0
msg_a20_ns: db "a20 ns", 0
memory_region_count: db 0
bdisk: db 0
bpart: db 0

[global bdisk]
[global bpart]
[global memory_region_count]

g_nulldesc:
    dd 0
    dd 0
g_codedesc:
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (mid)
    db 0b10011010   ; Flags
    db 0b11001111   ; Flags + Limit (high)
    db 0x00         ; Base (high)
g_datadesc:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0b10010010
    db 0b11001111
    db 0x00
g_end:

g_desc:
    gdt_size: dw g_end - g_nulldesc - 1
    gdt_start: dd g_nulldesc

codeseg equ g_codedesc - g_nulldesc
dataseg equ g_datadesc - g_nulldesc

[bits 32]
start_pm:
    mov ax, dataseg
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov esi, msg_pm
    call print_pm

    call detect_cpuid
    call detect_lmode
    call setup_identity_paging
    call edit_gdt

    jmp codeseg:start_lm

    jmp $

print_pm:
    mov edi, 0xb8000
    mov ah, 0xf
.pmploop:
    mov al, [esi]
    mov [edi], ax
    add di, 2
    inc esi
    cmp byte [esi], 0
    jnz .pmploop
    ret

detect_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid
    ret
.no_cpuid:
    mov si, msg_nocpuid
    call print_pm
    jmp $

detect_lmode:
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .nolmode
    ret
.nolmode:
    mov si, msg_nolmode
    call print_pm
    jmp $

[extern _end_all]
page_table_entry equ _end_all

setup_identity_paging:
    mov edi, page_table_entry
    mov cr3, edi
    mov dword [edi], _end_all + 0x1007
    add edi, 0x1000
    mov dword [edi], _end_all + 0x2007
    add edi, 0x1000
    mov dword [edi], _end_all + 0x3007
    add edi, 8
    mov dword [edi], _end_all + 0x4007
    add edi, 8
    mov dword [edi], _end_all + 0x5007
    add edi, 8
    mov dword [edi], _end_all + 0x6007
    sub edi, 24
    add edi, 0x1000
    mov ebx, 0x00000007
    mov ecx, 2048
    .set_entry:
        mov dword [edi], ebx
        add ebx, 0x1000
        add edi, 8
        loop .set_entry
    mov si, msg_made_pt
    call print_pm
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    mov eax, cr0
    or eax, 1<<31
    mov cr0, eax
    ret

edit_gdt:
    mov [g_codedesc + 6], byte 0b10101111
    mov [g_datadesc + 6], byte 0b10101111
    ret

msg_nolmode: db "No LMode", 0
msg_nocpuid: db "No CPUID", 0
msg_made_pt: db "Made Page Tables", 0
msg_pm: db "Entered PMode", 0

[bits 64]
%include "idt.asm"
%include "gdt.asm"
%include "task.asm"
[extern main]
[extern _stack]
[global start_lm]
start_lm:
    mov rbp, _stack
    mov rsp, rbp

    mov edi, 0xb8000
    mov rax, 0x1f201f201f201f20
    mov ecx, 500
    rep stosq
    
    call EnableSSE
    
    mov rbp, 0 ; empty stack frame
    call main
    jmp $

EnableSSE:
    mov rax, cr0
    and ax, 0b11111101
    or ax, 0b00000001
    mov cr0, rax

    mov rax, cr4
    or ax, 0b11000000
    mov cr4, rax
    ret

[global walk_stack]
walk_stack:
    push rbp
    mov rbp, rsp
    mov rbx, [rbp + 8] ; Old rbp
    mov rdi, [rbp + 16] ; arg 1
    mov rdx, [rbp + 20] ; arg 2
    xor rax, rax
.loop:
    test rbx, rbx
    jz .done
    mov rcx, [rbx + 8]
    mov rbx, [rbx]
    mov [rdi], rcx
    add rdi, 8
    inc rax
    cmp rax, rdx
    jl .loop
.done:
    mov rsp, rbp
    pop rbp
    ret

[global end_extboot]
end_extboot: dq $