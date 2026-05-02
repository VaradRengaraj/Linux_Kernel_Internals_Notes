// vaddr_inspect.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/sched.h>

#define PROC_NAME "vaddr_inspect"
#define BUF_SIZE 128

static struct proc_dir_entry *proc_entry;

static void dump_vmas(struct mm_struct *mm)
{
    struct vm_area_struct *vma;

    printk(KERN_INFO "---- VMA DUMP START ----\n");

    VMA_ITERATOR(vmi, mm, 0);

//    for (vma = mm->mmap; vma; vma = vma->vm_next) {
    for_each_vma(vmi, vma) {
        printk(KERN_INFO "VMA: %lx-%lx | flags: %lx\n",
               vma->vm_start, vma->vm_end, vma->vm_flags);
    }

    printk(KERN_INFO "---- VMA DUMP END ----\n");
}

static void inspect_address(struct mm_struct *mm, unsigned long addr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    printk(KERN_INFO "Inspecting VA: 0x%lx\n", addr);

    pgd = pgd_offset(mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_INFO "PGD not present\n");
        return;
    }
    printk(KERN_INFO "PGD: %px val=%lx\n", pgd, pgd_val(*pgd));

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        printk(KERN_INFO "P4D not present\n");
        return;
    }
    printk(KERN_INFO "P4D: %px val=%lx\n", p4d, p4d_val(*p4d));

    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_INFO "PUD not present\n");
        return;
    }
    printk(KERN_INFO "PUD: %px val=%lx\n", pud, pud_val(*pud));

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_INFO "PMD not present\n");
        return;
    }
    printk(KERN_INFO "PMD: %px val=%lx\n", pmd, pmd_val(*pmd));

    //if (pmd_huge(*pmd) || pmd_large(*pmd)) {
    //    printk(KERN_INFO "PMD maps a huge page\n");
    //    return;
    //}

    pte = pte_offset_kernel(pmd, addr);
    if (!pte) {
        printk(KERN_INFO "PTE null\n");
        return;
    }

    if (!pte_present(*pte)) {
        printk(KERN_INFO "PTE not present\n");
        //pte_unmap(pte);
        return;
    }

    printk(KERN_INFO "PTE: %px val=%lx\n", pte, pte_val(*pte));

    unsigned long pfn = pte_pfn(*pte);
    unsigned long phys = (pfn << PAGE_SHIFT) | (addr & ~PAGE_MASK);

    printk(KERN_INFO "PFN: %lx | PHYS: 0x%lx\n", pfn, phys);

    //pte_unmap(pte);
}

static ssize_t proc_write(struct file *file, const char __user *ubuf,
                          size_t count, loff_t *ppos)
{
    char buf[BUF_SIZE];
    struct mm_struct *mm = current->mm;

    if (count >= BUF_SIZE)
        return -EINVAL;

    if (copy_from_user(buf, ubuf, count))
        return -EFAULT;

    buf[count] = '\0';

    if (buf[0] == 'a') {
        dump_vmas(mm);
    } else {
        unsigned long addr;
        if (kstrtoul(buf, 0, &addr) == 0) {
            inspect_address(mm, addr);
        } else {
            printk(KERN_INFO "Invalid input\n");
        }
    }

    return count;
}

static const struct proc_ops proc_fops = {
    .proc_write = proc_write,
};

static int __init vaddr_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry) {
        printk(KERN_ERR "Failed to create /proc entry\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit vaddr_exit(void)
{
    proc_remove(proc_entry);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(vaddr_init);
module_exit(vaddr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Virtual Address Inspector");
