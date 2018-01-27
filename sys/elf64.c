#include <sys/defs.h>
#include <sys/elf64.h>
#include <sys/vmm.h>
#include <sys/tarfs.h>
#include <sys/utils.h>
#include <sys/kprintf.h>
#include <sys/string.h>
#include <sys/process.h>
#include <sys/mm_types.h>
#include <sys/fs.h>

//TODO: replace this with tarfs_open()
void *get_bin_addr(const char *fname)
{
    struct posix_header_ustar *tstart = (struct posix_header_ustar *) &_binary_tarfs_start;

    if(fname == NULL) {
        return NULL;
    }

    uint64_t *rp = (uint64_t *) tstart;

    while(rp[0] || rp[1] || rp[2]) {
        if(!strcmp(tstart->name, fname)) {
            return (void *) tstart;
        }
        else {
            uint64_t inum = atoi(tstart->size);
            inum = otod(inum);

            if(inum%512) inum = ((inum/512)*512)+512;

            tstart = (struct posix_header_ustar *) ((uint64_t) tstart + inum + sizeof(struct posix_header_ustar));

            rp = (uint64_t *) tstart;

        }
    }

    return NULL;
}

void allocate_heap(task_struct_t *task)
{
    vm_area_struct_t *vma = (vm_area_struct_t *) kmalloc(sizeof(vm_area_struct_t));

    vm_area_struct_t *vma_temp = task->mm->mmap;
    while(vma_temp->vm_next != NULL)
        vma_temp = vma_temp->vm_next;
    vma_temp->vm_next = vma;

    task->mm->brk = HEAP_START;
    vma->vm_start = HEAP_START;
    vma->vm_end   = HEAP_START + PAGE_SIZE;
    vma->vm_type  = VM_HEAP;
    vma->vm_mm    = task->mm;
    vma->vm_flags = (IS_RD + IS_WR);
    vma->file     = NULL;
    vma->vm_next  = NULL;

}

void allocate_stack(task_struct_t *task)
{
    vm_area_struct_t *vma = (vm_area_struct_t *) kmalloc(sizeof(vm_area_struct_t));

    vm_area_struct_t *vma_temp = task->mm->mmap;
    while(vma_temp->vm_next != NULL)
        vma_temp = vma_temp->vm_next;
    vma_temp->vm_next = vma;

    vma->vm_type  = VM_STACK;
    vma->vm_mm    = task->mm;
    vma->vm_start = STACK_TOP + PAGE_SIZE;
    vma->vm_end   = STACK_TOP;
    vma->vm_flags = (IS_RD + IS_WR);
    vma->file     = NULL;
    vma->vm_next  = NULL;

    task->mm->brk  = STACK_TOP;
    task->stack_p = (uint64_t) (STACK_TOP + PAGE_SIZE - 16);
}

void print_elf_info(task_struct_t *task)
{
    mm_struct_t *mm_temp = task->mm;
    vm_area_struct_t *vma_temp = task->mm->mmap;

    klog(INFO, "MM Struct: \n");
    klog(INFO, "RIP: %p, PML4: %p, Start Code: %p, End Code: %p, Start Data: %p, End Data: %p\n",
               task->rip, mm_temp->pml4, mm_temp->start_code, mm_temp->end_code,
               mm_temp->start_data, mm_temp->end_data);

    while(vma_temp != NULL) {
        klog(INFO, "VM Area Struct: \n");
        klog(INFO, "VM_START: %p, VM_END: %p, VM_FLAGS: %p\n", vma_temp->vm_start, vma_temp->vm_end, vma_temp->vm_flags);
        vma_temp = vma_temp->vm_next;
    }
}

int load_elf(task_struct_t *task, const char *fname)
{

    struct posix_header_ustar * p_hdr_ustar = (struct posix_header_ustar *) get_bin_addr(fname);
    if(p_hdr_ustar == NULL)
        return -1;

    Elf64_Ehdr *e_hdr = (Elf64_Ehdr *) (p_hdr_ustar + 1);


    Elf64_Phdr *p_hdr = (Elf64_Phdr *) ((uint64_t)e_hdr + e_hdr->e_phoff);
    task->rip = e_hdr->e_entry; //TODO: should be assigned to stack[510], i dont' need rip then

    task->mm = (mm_struct_t *) kmalloc(sizeof(mm_struct_t));

    for(int i = 0; i < e_hdr->e_phnum; i++)
    {
        if(p_hdr->p_type == 1) {
            vm_area_struct_t *vma = (vm_area_struct_t *) kmalloc(sizeof(vm_area_struct_t));
            memset(vma, 0, sizeof(vm_area_struct_t));

            vma->vm_mm    = task->mm;
            vma->vm_start = p_hdr->p_vaddr;
            vma->vm_end   = p_hdr->p_vaddr + p_hdr->p_memsz;
            vma->vm_flags = p_hdr->p_flags;
            vma->file     = NULL;
            vma->vm_next  = NULL;

            if(task->mm->mmap == NULL)
                task->mm->mmap = vma;
            else {
                vm_area_struct_t *vma_temp = task->mm->mmap;
                while(vma_temp->vm_next != NULL)
                    vma_temp = vma_temp->vm_next;
                vma_temp->vm_next = vma;
            }

            if(vma->vm_flags == (IS_RD + IS_XE))
            {
                task->mm->start_code = vma->vm_start;
                task->mm->end_code   = vma->vm_end;

                vma->file = (file_t *) kmalloc(sizeof(file_t));
                vma->file->f_start = (uint64_t) e_hdr;
                vma->file->f_pgoff = p_hdr->p_offset;
                vma->file->f_size  = p_hdr->p_filesz;
            }
            else if(vma->vm_flags == (IS_RD + IS_WR)) { //TODO: check this condition.
                task->mm->start_data = vma->vm_start;
                task->mm->end_data   = vma->vm_end;

                vma->file = (file_t *) kmalloc(sizeof(file_t));
                vma->file->f_start = (uint64_t) e_hdr;
                vma->file->f_pgoff = p_hdr->p_offset;
                vma->file->f_size  = p_hdr->p_filesz;
            }
        }
        p_hdr++;
    }

    allocate_heap(task);
    allocate_stack(task);
    print_elf_info(task);

    return 0;
}
