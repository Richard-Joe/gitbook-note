#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/pgtable.h>
#include <asm/page_types.h>
#include <asm/sections.h>
#include <asm/kasan.h>

static int pid;
static unsigned long va;

module_param(pid, int, 0644); // process pid
module_param(va, ulong, 0644); // process virtuall address

static void find_pgd(void) {
    // PAGE_OFFSET 表示在内核虚拟地址空间中页表开始的地址
    printk("PAGE_OFFSET        = 0x%lx", PAGE_OFFSET);
    //printk("high_memory     = 0x%p", high_memory);
    printk("VMALLOC_START      = 0x%lx", VMALLOC_START);
    printk("VMALLOC_END        = 0x%lx", VMALLOC_END);
    printk("VMALLOC_SIZE_TB    = %lu", VMALLOC_SIZE_TB);
    printk("VMEMMAP_START      = 0x%lx", VMEMMAP_START);

    //printk("KASAN_SHADOW_START = 0x%lx", KASAN_SHADOW_START);
    //printk("KASAN_SHADOW_END   = 0x%lx", KASAN_SHADOW_END);

    printk("CPU_ENTRY_AREA_BASE= 0x%lx", CPU_ENTRY_AREA_BASE);
    printk("ESPFIX_BASE_ADDR   = 0x%lx", ESPFIX_BASE_ADDR);
    //printk("ESPFIX_STACK_SIZE   = %lu", ESPFIX_STACK_SIZE);

    printk("__START_KERNEL_map = 0x%lx", __START_KERNEL_map);

    printk("MODULES_VADDR      = 0x%lx", MODULES_VADDR);
    printk("MODULES_END        = 0x%lx", MODULES_END);
    printk("MODULES_LEN        = %lu", MODULES_LEN);

    printk("VSYSCALL_ADDR      = 0x%lx", VSYSCALL_ADDR);

    printk("FIXADDR_START      = 0x%lx", FIXADDR_START);
    printk("FIXADDR_TOP        = 0x%lx", FIXADDR_TOP);
    printk("FIXADDR_SIZE       = %u", FIXADDR_SIZE);

    // *_SHIFT表示对数，最终反馈的是大小
    // 比如：计算页大小 PAGE_SIZE = 1 << PAGE_SHIFT
    printk("PGDIR_SHIFT     = %d   ", PGDIR_SHIFT);
    printk("PGDIR_SIZE      = %lu  ", PGDIR_SIZE); // 全局页（PGD）目录表中一个单独表项所能映射区域的大小
    printk("P4D_SHIFT       = %d   ", P4D_SHIFT);
    printk("P4D_SIZE        = %lu  ", P4D_SIZE);   // P4D 目录表中一个单独表项所能映射区域的大小
    printk("PUD_SHIFT       = %d   ", PUD_SHIFT);
    printk("PUD_SIZE        = %lu  ", PUD_SIZE);   // PUD 目录表中一个单独表项所能映射区域的大小
    printk("PMD_SHIFT       = %d   ", PMD_SHIFT);
    printk("PMD_SIZE        = %lu  ", PMD_SIZE);   // PMD 目录表中一个单独表项所能映射区域的大小
    printk("PTE_SHIFT       = %d   ", PTE_SHIFT);  // 页表大小对应的对数
    printk("PAGE_SHIFT      = %d   ", PAGE_SHIFT);
    printk("PAGE_SIZE       = %lu  ", PAGE_SIZE);  // 一个页大小

    // mask
    printk("PGDIR_MASK      = 0x%lx", PGDIR_MASK);
    printk("P4D_MASK        = 0x%lx", P4D_MASK);
    printk("PUD_MASK        = 0x%lx", PUD_MASK);
    printk("PMD_MASK        = 0x%lx", PMD_MASK);
    printk("PAGE_MASK       = 0x%lx", PAGE_MASK);

    // PTRS_PER_PGD 用于表示页全局目录（PGD）中页表项的数量
    printk("PTRS_PER_PGD    = %d   ", PTRS_PER_PGD); // PGD 表项数量
    printk("PTRS_PER_P4D    = %d   ", PTRS_PER_P4D); // P4D 表项数量
    printk("PTRS_PER_PUD    = %d   ", PTRS_PER_PUD); // PUD 表项数量
    printk("PTRS_PER_PMD    = %d   ", PTRS_PER_PMD); // PMD 表项数量
    printk("PTRS_PER_PTE    = %d   ", PTRS_PER_PTE); // PTE 表项数量

}

static void v2p(struct mm_struct *mm, unsigned long va) {
    unsigned long pa = 0; // physics address
    pgd_t *pgd = NULL;
    p4d_t *p4d = NULL;
    pud_t *pud = NULL;
    pmd_t *pmd = NULL;
    pte_t *pte = NULL;

    pgd = pgd_offset(mm, va);
    printk("pgd = 0x%p\n", pgd);
    printk("pgd_val = 0x%lx,   pgd_index = %lu\n", pgd_val(*pgd), pgd_index(va));
    if (pgd_none(*pgd)) {
        printk("not mapped in pgd\n");
        return;
    }

    p4d = p4d_offset(pgd, va);
    printk("p4d = 0x%p\n", p4d);
    printk("p4d_val = 0x%lx,   p4d_index = %lu\n", p4d_val(*p4d), p4d_index(va));
    if (p4d_none(*p4d)) {
        printk("not mapped in p4d\n");
        return;
    }

    pud = pud_offset(p4d, va);
    printk("pud = 0x%p\n", pud);
    printk("pud_val = 0x%lx,   pud_index = %lu\n", pud_val(*pud), pud_index(va));
    if (pud_none(*pud)) {
        printk("not mapped in pud\n");
        return;
    }

    pmd = pmd_offset(pud, va);
    printk("pmd = 0x%p\n", pmd);
    printk("pmd_val = 0x%lx,   pmd_index = %lu\n", pmd_val(*pmd), pmd_index(va));
    if (pmd_none(*pmd)) {
        printk("not mapped in pmd\n");
        return;
    }

    pte = pte_offset_kernel(pmd, va);
    printk("pte = 0x%p\n", pte);
    printk("pte_val = 0x%lx,   pte_index = %lu\n", pte_val(*pte), pte_index(va));
    if (pte_none(*pte)) {
        printk("not mapped in pte\n");
        return;
    }
    if (!pte_present(*pte)) {
        printk("pte not in RAM\n");
        return;
    }

    pa = (pte_val(*pte) & PAGE_MASK);
    printk("virtual address 0x%lx --> physics address 0x%lx\n", va, pa);

}

static void kernel_test(void) {
    void *kptr = NULL, *vptr = NULL;
    struct page *pg = NULL;

    kptr = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (kptr == NULL) {
        return;
    }
    printk(KERN_INFO "kmalloc allocated memory at 0x%lx\n", (unsigned long)kptr);
    printk(KERN_INFO "Physical address: 0x%lx\n", (unsigned long)virt_to_phys(kptr));
    printk(KERN_INFO "Physical address: 0x%lx\n", (unsigned long)kptr - PAGE_OFFSET);

    kfree(kptr);

    vptr = vmalloc(PAGE_SIZE);
    if (vptr == NULL) {
        return;
    }
    printk(KERN_INFO "vmalloc allocated memory at 0x%lx\n", (unsigned long)vptr);
    pg = vmalloc_to_page(vptr);
    if (pg == NULL) {
        vfree(vptr);
        return;
    }
    printk(KERN_INFO "Physical address: 0x%llx\n", page_to_phys(pg));
    vfree(vptr);

    //alloc_page();
    //__get_free_pages(GFP_KERNEL, 1);
}

static void user_test(void) {
    struct task_struct *pcb = NULL; // process task struct
    struct pid *p = NULL;

    p = find_vpid(pid);
    if (p == NULL)
        return;
    pcb = pid_task(p, PIDTYPE_PID);
    if (pcb == NULL)
        return;
    if (!find_vma(pcb->mm, va)) {
        printk("virtual address 0x%lx is not available\n", va);
        return;
    }

    v2p(pcb->mm, va);
}

static int v2p_init(void) {
    printk(KERN_ALERT "Hello, V2P\n");
    find_pgd();
    //user_test();
    kernel_test();
    return 0;
}

static void v2p_exit(void) {
    printk(KERN_ALERT "Googbye, V2P\n");
}

module_init(v2p_init);
module_exit(v2p_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("virtual address to physics address");


