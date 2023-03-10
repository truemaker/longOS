#pragma once
#include <typedef.h>
enum task_state {
    proc_idle,
    proc_running,
    proc_blocked,
    proc_new,
    proc_exit
};
typedef struct proc {
    uint64_t rsp;
    uint64_t cr3;
    proc* next;
    bool first;
    uint64_t pid;
    uint8_t state;
    uint64_t childs;
    uint64_t* child_pids;
    uint64_t ppid;
    uint8_t quantum;
    uint8_t current_quantum;
} proc_t;
extern "C" void _task_switch(uint64_t* current_rsp_store, uint64_t* new_task_rsp);
extern bool in_kernel;
void switch_task(void);
void yield(void);
void init_task(void);
proc_t* fork(void* entry);
void quit(void);