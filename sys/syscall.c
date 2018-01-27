#include <sys/defs.h>
#include <sys/vmm.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/string.h>
#include <sys/utils.h>
#include <sys/asm_utils.h>
#include <sys/fs.h>
#include <sys/elf64.h>
#include <sys/terminal.h>
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/user.h>
#include <sys/env.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/gdt.h>

#include <dirent.h>
#include<logger.h>
#include<sys/time.h>


extern void fork_return(void);
extern char PWD[MAX_NAME+1];
extern string users[5];
void sys_exit()
{
	//print_task_list();

    //task_struct_t *temp = curr_task;
    //add_to_zombie_queue(curr_task);
    remove_from_queue(curr_task);
}

uint64_t sys_fork()
{
    task_struct_t *child_task;

    uint64_t *stack = (uint64_t *) kmalloc(20*PAGE_SIZE);
    if(!stack) {
        kpanic("Not able to allocate stack!!");
    }

    child_task = (task_struct_t *) kmalloc(PAGE_SIZE);
    child_task->pml4 = (uint64_t) kmalloc(PAGE_SIZE) - KERNAL_BASE_ADDRESS;
    child_task->mm = (mm_struct_t *) kmalloc(PAGE_SIZE);

    child_task->stack_p = child_task->kern_stack = (uint64_t) &stack[510];

    get_system_uptime(child_task->start_time);
    child_task->fdoffset = 4;
    child_task->state    = TASK_RUNNABLE;
    child_task->pid      = get_pid();
    child_task->ppid     = curr_task->pid;
    child_task->sibling  = NULL;
    strcpy(child_task->pcmd_name, curr_task->pcmd_name);
    strconcat(child_task->pcmd_name, "_child");
    prev_task = curr_task;

    //Set up virtual memory.
    setup_child_ptables(child_task->pml4);

    curr_task->child      = child_task;
    child_task->parent    = curr_task;
    add_to_queue(child_task);

    //Deep copy
    uint64_t * chld_stack = (uint64_t *) child_task->kern_stack;
    uint64_t * curr_stack = (uint64_t *) curr_task->kern_stack;
    memcpy(chld_stack-510, curr_stack-510, 4080); //TODO: debug this.
    memcpy(child_task->fd, curr_task->fd, sizeof(fd_t) * MAX_FILES);
    memcpy(child_task->mm, curr_task->mm, sizeof(mm_struct_t));
    if(curr_task->mm->mmap != NULL) {
        vm_area_struct_t *curr_vma = curr_task->mm->mmap;
        vm_area_struct_t *chld_vma = (vm_area_struct_t *) kmalloc(PAGE_SIZE);
        memcpy(chld_vma, curr_vma, sizeof(vm_area_struct_t));
        child_task->mm->mmap = chld_vma;
        while(curr_vma->vm_next != NULL) {
            curr_vma = curr_vma->vm_next;
            chld_vma->vm_next = (vm_area_struct_t *) kmalloc(PAGE_SIZE);
            chld_vma = chld_vma->vm_next;
            memcpy(chld_vma, curr_vma, sizeof(vm_area_struct_t));
        }
        chld_vma->vm_next = NULL;
    }
    else {
       child_task->mm->mmap = NULL;
    }

    /*
    //copying user stack
    uint64_t page_addr = (uint64_t) kmalloc(PAGE_SIZE);
    uint64_t page_addr2 = (uint64_t) kmalloc(PAGE_SIZE);
    uint64_t falign_addr = STACK_TOP-4096;
    uint64_t falign_addr2 = STACK_TOP;
    memcpy((void *) page_addr, (void *) falign_addr, PAGE_SIZE);
    memcpy((void *) page_addr2, (void *) falign_addr2, PAGE_SIZE);
    page_addr = page_addr - KERNAL_BASE_ADDRESS;
    page_addr2 = page_addr2 - KERNAL_BASE_ADDRESS;
    write_cr3(child_task->pml4);
    map_proc(page_addr, falign_addr);
    map_proc(page_addr2, falign_addr2);
    write_cr3(curr_task->pml4);
    */

    //volatile uint64_t rip_loc;
    //curr_task = prev_task;

    //TODO: make these are functions in asm utils.h
    //__asm__ __volatile__("movq $2f, %0;" "2:\t" : "=g"(rip_loc));
    //volatile uint64_t stack_loc;
    //__asm__ __volatile__("movq %%rsp, %0" : "=r"(stack_loc));

    //__asm__ __volatile__("movq $2f, %0;" "2:\t" : "=g"(child_task->rip));
    //__asm__ __volatile__("movq %%rsp, %0" : "=r"(child_task->stack_p));

    /*
    if(curr_task == prev_task) {
      kprintf("In Parent!\n");
      child_task->kern_stack = child_task->kern_stack - 128;
      return child_task->pid;
    }
    else {
      kprintf("In Child!\n");
      return 0;
    }
    */

    child_task->kern_stack = child_task->kern_stack - 80;
    child_task->stack_p = child_task->kern_stack;
    child_task->rip = (uint64_t) fork_return;
    schedule();
    return child_task->pid;

}

uint64_t sys_execve(char *fname, char *argv[], char *envp[])
{
    task_struct_t *new_task = curr_task;
    int * retp = (int *) get_bin_addr(fname);
    if(retp != NULL)
        klog(INFO, "Loading %s was sucessfull\n", fname);
    else {
        klog(ERR, "Error loading %s\n", fname);
        return -1;
    }

    uint64_t * stack = (uint64_t *) kmalloc(PAGE_SIZE);
    if(!stack) {
        kpanic("Not able to allocate stack for init\n");
    }

    char args[5][64]; //TODO: can this be less restrictive. Can use better version of kmalloc()
    int arg_cnt = 0;

    while(argv[arg_cnt] != NULL && arg_cnt < 5) {
           args[arg_cnt][0] = '\0';
           strcpy(args[arg_cnt], argv[arg_cnt]);
           arg_cnt++;
    }
    for(int i = arg_cnt; i < 5; i++) args[i][0] = '\0';

    strcpy(new_task->pcmd_name, fname);

    //TODO: setup user address space.
    struct page_map_level_4* old_pml4 = (struct page_map_level_4*) read_cr3();
    struct page_map_level_4* new_pml4 = (struct page_map_level_4*) kmalloc(PAGE_SIZE);

    old_pml4 = (struct page_map_level_4 *)((uint64_t) old_pml4 + KERNAL_BASE_ADDRESS); //TODO: use pa_to_va

    new_pml4->pml4e[511] = old_pml4->pml4e[511];

    new_task->pml4 = (uint64_t) new_pml4 - KERNAL_BASE_ADDRESS;

    write_cr3(new_task->pml4);

    //char bin_name[64];
    //strcpy(bin_name, PWD+1);
    //strconcat(bin_name, args[0]);
    //klog(IMP, "bin_name: %s\n", bin_name);
    //load_elf(new_task, bin_name);

    //Load process.
    load_elf(new_task, args[0]);

    //TODO: do for envp too.
    set_tss_rsp((void *) curr_task->kern_stack);
    uint64_t args_user = STACK_TOP + 0x1000 - 0x10 - sizeof(args);
    write_cr3(curr_task->pml4);
    memcpy((void *) args_user, (void *) args, sizeof(args));
    for(int i = arg_cnt; i > 0; i--)
       *(uint64_t *)(args_user - 8*i) = args_user + (arg_cnt-i)*64;
    *(uint64_t *)(args_user - 8*(arg_cnt+1)) = arg_cnt;

    args_user = args_user - 8*(arg_cnt+1);
    new_task->stack_p = args_user;

    switch_to_userspace(new_task);

    return -1;
}

uint64_t sys_read(uint64_t fd, void* addr, uint64_t size)
{
	//klog(INFO,"Inside sys read start: %d ,  %p\n", fd, addr);

    if(fd == STDIN) {
        term_read((uint64_t)addr, size);
        return 1;
    }else{
    			file_node_t* fnode_ptr  = (file_node_t*)curr_task->fd[fd];

    			if( fnode_ptr->fseek < fnode_ptr->fsize){
    				//debug("\nIn read : size : %d, seek : %d.", fnode_ptr->fsize, fnode_ptr->fseek);
    				//klog(INFO,"In read : fd : %d ,  seek : %d,", fd, fnode_ptr->fseek, fnode_ptr->fsize);
    				strcpy(addr,fnode_ptr->private_data + fnode_ptr->fseek);
    				fnode_ptr->fseek++;
    				return 1;
    			}
    			//TODO. init other values in fnode_ptr. At this point .I don't care.
    			return -1;
    }
    //klog(INFO,"Inside syscall read end: %d ,  %p\n", fd, addr);
}

uint64_t sys_write(uint64_t fd, uint64_t addr, uint64_t size)
{
    if(fd == STDOUT || fd == STDERR) {
        term_write(addr, size);
    }

    return 1;
}

uint64_t sys_waitpid(uint64_t pid, uint64_t status, uint64_t options)
{
    curr_task->state = TASK_WAITING;

    if(zombie_task->pid == pid) return 0;

    schedule();

    return 0;
}

uint64_t sys_getdents(uint64_t fd, struct dirent *dir, uint64_t size)
{

    char buf[512] = {0};
    strcpy(buf, PWD);
    uint32_t child_fidx ;
    child_fidx = get_child(fd, dir->offset);
    if(child_fidx == -1){
      dir->offset = -1;
      return -1;
    }



    strcpy(dir->d_name, tarfs_fds[child_fidx].name);// copy name
    dir->inode = child_fidx;
    dir->offset = child_fidx;
    dir->type = tarfs_fds[child_fidx].type;
    dir->size = tarfs_fds[child_fidx].size;
    strcpy(dir->fowner , users[tarfs_fds[child_fidx].fnode->fowner]);
    //dir->fowner = tarfs_fds[child_fidx].fnode->fowner;
    klog(INFO,"%s\n", dir->d_name);
    return child_fidx;
}

uint64_t sys_dup2(uint64_t old_fd, uint64_t new_fd)
{
    //TODO:
    return 0;
}

uint64_t sys_pipe(uint64_t* fds)
{
    //TODO:
    return 0;
}

uint64_t sys_getcwd(char *buf, uint64_t size)
{
  //klog(INFO,"Inside sys_getcwd\n");
	strcpy(buf ,PWD);
	//klog(INFO,"cwd: %s\n", cwd);
	klog(INFO,"Buff from getcwd      : %s\n", buf);
	return 0;
}

uint64_t sys_access(char * pathname, uint64_t mode)
{
	klog(INFO,"Inside sys_chdir old  : %s \n", PWD);
	klog(INFO,"Inside sys_chdir new  : %s \n", pathname);
	for(uint32_t idx = 0 ;idx< 500 ; idx++){
		if(!strcmp((const char *)tarfs_fds[idx].name, pathname)){
			if(tarfs_fds[idx].type == DIRTYPE){
				strcpy(PWD, pathname);
				klog(INFO,"Change dir to         : %s\n",pathname);

			}
			else{
				klog(INFO,"cd: %s: No such file or directory\n",pathname);
			}
			return 0;
		}
	}


	//PWD = (char *)pathname;
	klog(INFO,"Not found Change dir :  %s\n",pathname);
	return 0;

    return 0;
}

uint64_t sys_chdir(char * pathname)
{
	//klog(INFO,"Inside sys_chdir old  : %s \n", cwd);
	//klog(INFO,"Inside sys_chdir new  : %s \n", pathname);
	for(uint32_t idx = 0 ;idx< 500 ; idx++){
		if(!strcmp((const char *)tarfs_fds[idx].name, pathname)){
			if(tarfs_fds[idx].type == DIRTYPE){
				strcpy(PWD, pathname);
				//klog(INFO,"Change dir to         : %s\n",pathname);
                return 0;
			}
			else{
				kprintf("cd: %s: No such file or directory\n",pathname);
			}

		}
	}


	//cwd = (char *)pathname;
		kprintf("cd: %s: No such file or directory\n",pathname);
	return 0;
}

uint64_t sys_open(char * pathname, uint64_t flags){
	//read_rtc();
	klog(INFO,"syscall : sys_open() , %d\n", curr_task->pid);

	int fidx = get_index_by_name(pathname);

	if(O_DIRECTORY  & flags){
		klog(INFO,"Open dir.\n");
		return fidx;
	}
	if(fidx == -1){
		return ENOTDIR;
	}else{

		klog(INFO,"Open file.\n");
		file_node_t* fnode_ptr  = (file_node_t*)kmalloc(PAGE_SIZE);
		fnode_ptr->fmode = 0;
		fnode_ptr->fseek = 0;
		fnode_ptr->fsize = tarfs_fds[fidx].size;
		fnode_ptr->private_data = (void*)tarfs_fds[fidx].data;
		//TODO. init other values in fnode_ptr. At this point .I don't care.
		curr_task->fdoffset++;
		curr_task->fd[curr_task->fdoffset] = fnode_ptr;

		klog(INFO,"fname : %s ,  flags : %p, found at idx : %d, fdoffset :%d\n", pathname, flags, fidx, curr_task->fdoffset);
	}
	return curr_task->fdoffset; //return offset in curr task. so that calling process can acccess it using fdoffset.
}

uint64_t sys_close(uint64_t fd)
{



	curr_task->fd[fd] = NULL;

    return 0;
}

void sys_sched_yield()
{
    schedule();
}

void sys_shutdown(uint64_t code)
{
    //TODO: clearn all tasks.
    sys_exit();
}

uint64_t sys_mmap(void *start, uint64_t length, int32_t prot,
        int32_t flags, int32_t fd, uint64_t offset){


	klog(INFO,"length =: %d\n", length);
	vm_area_struct_t * mmap = curr_task->mm->mmap;
	while(mmap !=NULL && mmap->vm_type !=VM_HEAP){
		//klog(INFO,"addr :  %p\n", mmap->vm_start);
		//klog(INFO,"addr :  %p\n", mmap->vm_end);
		mmap = mmap->vm_next;
	}

	if(mmap ==NULL){
		klog(INFO,"mmap is null\n");
	}
	if(mmap->vm_start + length < mmap->vm_end){
		uint64_t vm_start = mmap->vm_start;
		mmap->vm_start = mmap->vm_start + length;
		klog(INFO,"vm_start : %p. type: %d.\n", vm_start, mmap->vm_type);
		return mmap->vm_start - KERNAL_BASE_ADDRESS;
	}
	klog(INFO,"malloc is null\n");
    return 0;

}
uint64_t sys_fstat(int fidx, fstat_t* statbuf){
	klog(INFO,"Inside sys_fstat*****\n");
	memset(statbuf,  '\0', sizeof(statbuf));
	statbuf->st_size  = tarfs_size(fidx);
	//statbuf->st_size  = tarfs_owner(fidx);
	return (uint64_t)statbuf;

}

uint64_t sys_lseek(uint32_t fildes, uint64_t offset, uint32_t whence){
    //TODO : handle rootdir. idc not required not working.
	klog(INFO,"lseek offset :  %d\n", offset);
	tarfs_fds[fildes].offset = offset;
    return 0;
}

uint64_t sys_ps(){
	//debug("");
	print_task_list();
	//if(buff){}
	return 0;
}

uint64_t sys_gettimeofday(struct tm* tm_time){
	read_rtc(tm_time);


	return 1;
}
uint64_t sys_clear_term(){

    clr_term();
    return 0;
}

uint64_t sys_setuid( uint64_t user_id){
    if(user_id == -1){ //if -1 then get_id called; dirty hack.
        return get_uid();
    }
    klog(INFO,"Inside  sys_setuid :  %d \n", user_id);
    set_uid(user_id);
    return user_id;

}
