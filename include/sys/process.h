#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#include <sys/fs.h>
#include <sys/mm_types.h>

#define MAX_NAME      150
#define MAX_FILES     50
#define MAX_PROCESS   1000
#define MAX_PID       (4 * 1024 * 1024)

enum state {
    TASK_RUNNING  = 1,
    TASK_RUNNABLE = 2,
    TASK_BLOCKED  = 3,
    TASK_SLEEPING = 4,
    TASK_WAITING  = 5,
    TASK_ZOMBIE   = 6
};

typedef struct task_struct {
    int pid;
    int ppid;
    int state;
    char start_time[12];
    int exit_code;

    mm_struct_t *mm;
    file_node_t *fd[MAX_FILES];
    uint32_t fdoffset;
    struct task_struct *next_task, *prev_task;
    struct task_struct *next_run, *prev_run;
    struct task_struct *parent, *sibling, *child;

    uint64_t pml4;
    uint64_t rip;
    uint64_t stack_p;
    uint64_t kern_stack;
    uint64_t sleep_t;

    char pcmd_name[MAX_NAME];
    char cdir_name[MAX_NAME];

} task_struct_t;

task_struct_t* init_proc(const char *name, int type);
void context_switch(task_struct_t *prev_task, task_struct_t *next_task);
pid_t get_pid();
void schedule();
void switch_to_userspace(task_struct_t *task);
void add_to_queue(task_struct_t *task);
void add_to_zombie_queue(task_struct_t *task);
void remove_from_queue(task_struct_t *task);
void delete_task(task_struct_t *task);
void print_task_list();

task_struct_t *curr_task;   //Running Task queue
task_struct_t *zombie_task; //Zombie Task queue
task_struct_t *init_task;
task_struct_t *kern_task;
task_struct_t *prev_task;

extern uint64_t kern_stack;
extern uint64_t user_stack;

uint64_t pages_used;
uint64_t total_pages;

#endif
