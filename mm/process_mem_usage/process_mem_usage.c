#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>

#define PROC_NAME "process_mem_usage"

/* ========================= */
/* seq_file show function    */
/* ========================= */

#define KiB(x) (x << (PAGE_SHIFT -10))

static int mem_usage_show(struct seq_file *m, void *v)
{
    struct task_struct *task;
    struct vm_area_struct *vma;
    struct vma_iterator vmi;
    struct mm_struct *mm;
    unsigned long rss, size, hiwater_size, hiwater_rss, text, data, stack, lib, shared, total_exec, shared_pages;

    seq_printf(m, "PID\tVmPeak(KB)\tVmSize(KB)\tVmHWM(KB)\tVmRss(KB)\tVmText(KB)\tVmData(KB)\tVmStack(KB)\tVmExe(KB)\nVmLib(KB)\n");

    for_each_process(task) {
        //struct mm_struct *mm;
        //unsigned long rss, size, text, data, stack, lib, shared, total_exec, shared_pages;

        mm = get_task_mm(task);
        if (!mm)
            continue;

        VMA_ITERATOR(vmi, mm, 0);
        rss = hiwater_rss =  KiB(get_mm_rss(mm)); //get_mm_rss(mm) << (PAGE_SHIFT - 10); 
        size = hiwater_size = KiB(mm->total_vm);
        text = (PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK)) >> 10;
        stack = KiB(mm->stack_vm);
        
        if(hiwater_size < KiB(mm->hiwater_vm))
            hiwater_size = KiB(mm->hiwater_vm);

        if(hiwater_rss < KiB(mm->hiwater_rss))
            hiwater_rss = KiB(mm->hiwater_rss);

        //shared = KiB(mm->shared_vm);       
        //data = size - text - shared - stack; //total size - text_size - shared_vm - stack_vm 

        mmap_read_lock(mm);
        shared_pages = 0;

        //for(vma = mm->mmap; vma; vma = vma->vm_next)
        for_each_vma(vmi, vma)
        {
            if(vma->vm_flags & VM_SHARED)
            {
                shared_pages += (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
            }            
        }

        mmap_read_unlock(mm);       

        shared = KiB(shared_pages);

        total_exec = KiB(mm->exec_vm);
        lib = total_exec - text; 
        seq_printf(m, "%d\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n", 
                         task->pid, hiwater_size, size, hiwater_rss, rss, text, data, stack, total_exec, lib);

        mmput(mm);
    }
    
    return 0;
}

/* ========================= */
/* open function             */
/* ========================= */

static int mem_usage_open(struct inode *inode, struct file *file)
{
    return single_open(file, mem_usage_show, NULL);
}

/* ========================= */
/* proc ops                  */
/* ========================= */

static const struct proc_ops mem_usage_fops = {
    .proc_open = mem_usage_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

/* ========================= */
/* init / exit               */
/* ========================= */

static int __init mem_usage_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &mem_usage_fops);
    pr_info("/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit mem_usage_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(mem_usage_init);
module_exit(mem_usage_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Process memory usage via /proc");

