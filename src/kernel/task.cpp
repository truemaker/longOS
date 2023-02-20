#include <task.h>
#include <heap.h>
#include <memory.h>
#include <vga.h>
#include <asm.h>
proc_t main_proc;
proc_t* current;
bool task_switch_in_progress;
bool task_creation_in_progress;
uint64_t next_pid;

void _stack_error() {
    print("[TASK] Stack error\n\r");
    // TODO: Destroy process
    for (;;);
}

#define PUSH(x,y) *x = y; x--;
uint64_t _create_task_stack(uint64_t base, uint64_t pml4, uint64_t entry) {
    uint64_t* value = (uint64_t*)base;
    PUSH(value,(uint64_t)&_stack_error);
    PUSH(value,entry);  // RIP
    PUSH(value,base);   // RBP
    PUSH(value,0);      // RAX
    PUSH(value,0);      // RBX
    PUSH(value,0);      // RCX
    PUSH(value,0);      // RDX
    PUSH(value,0);      // RSI
    PUSH(value,0);      // RDI
    PUSH(value,0);      // R8
    PUSH(value,0);      // R9
    PUSH(value,0);      // R10
    PUSH(value,0);      // R11
    uint64_t flags = 0;
    asm volatile("pushfq;pop %0" : "=a"(flags));
    PUSH(value,flags);  // RFLAGS
    return (uint64_t)value;
}

void _create_task(proc_t* proc, void* entry) {
    while (task_creation_in_progress) yield();
    task_creation_in_progress = true;
    proc->child_pids = 0;
    proc->childs = 0;
    proc->current_quantum = 0;
    proc->first = false;
    proc->next = main_proc.next;
    proc->pid = next_pid++;
    proc->ppid = current->pid;
    proc->quantum = 10;
    proc->rsp = (uint64_t)request_page();
    g_PTM->map((void*)proc->rsp,(void*)proc->rsp);
    debugf("Creating task stack\n\r");
    proc->rsp = _create_task_stack(proc->rsp+0x1000-8,(uint64_t)g_PTM->pml4,(uint64_t)entry)+8;
    debugf("Created task stack\n\r");
    debugf("Creating task cr3\n\r");
    proc->cr3 = read_cr3();
    debugf("Created task cr3");
    proc->state = proc_new;
    main_proc.next = proc;
    task_creation_in_progress = false;
    proc->state = proc_idle;
}

void init_task() {
    main_proc.first = true;
    main_proc.next = &main_proc;
    main_proc.child_pids = 0;
    main_proc.childs = 0;
    main_proc.current_quantum = 20;
    main_proc.pid = 0;
    main_proc.ppid = -1;
    main_proc.quantum = 20;
    main_proc.rsp = 0;
    main_proc.cr3 = read_cr3();
    main_proc.state = proc_running;
    task_switch_in_progress = false;
    current = &main_proc;
    yield();
}

void yield() {
    debugf("Yielding...\n\r");
    switch_task();
}

void switch_task() {
    task_switch_in_progress = true;
    proc_t* last = current;
    current = current->next;
    while (current->state != proc_idle) {
        if (current->state == proc_running) break;
        current = current->next;
    }
    if (current->state != proc_running) {
        last->state = proc_idle;
        current->state = proc_running;
        debugf("Initiating switch\n\r");
        _task_switch(&last->rsp,&current->rsp);
        debugf("Switch done\n\r");
    }
    task_switch_in_progress = false;
}

proc_t* fork(void* entry) {
    proc_t* task = (proc_t*)heap::malloc(sizeof(proc_t));
    _create_task(task,entry);
    return task;
}