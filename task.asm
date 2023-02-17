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
    
    mov rax, [rbp+24]  ; Load argument 'current_rsp'
    mov [rax], rsp    ; Save 'rsp'
    ;
    mov rax, [rbp+16] ; Load argument 'new_task_rsp'
    mov rsp, rax    ; Load 'rsp'
    
    ;mov rbx, [rax+8]  ; Load 'cr3'
    ;mov cr3, rbx      ; Apply 'cr3'
    
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