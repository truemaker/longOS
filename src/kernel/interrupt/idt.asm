[extern _idt]
[global idt_desc]
idt_desc:
    dw 4095
    dq _idt

[extern isr1_handler]
isr1:
    push rax
    push rbx
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    pushf

    call isr1_handler
    
    popf
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq
[global isr1]

[extern timer_handler]
timer:
    push rax
    push rbx
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    pushf

    call timer_handler
    
    popf
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq
[global timer]