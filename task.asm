[bits 64]
_task_switch:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    pushfq
    
    mov rdx, rdi        ; Load argument 'current_rsp'
    mov [rdx], rsp      ; Save 'rsp'
    
    mov rdx, rsi        ; Load argument 'new_task_rsp'
    mov rsp, [rdx]      ; Load 'rsp'
    
    popfq
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    ret
[global _task_switch]