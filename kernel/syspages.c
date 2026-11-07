#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

void
print_pte_flags(pte_t pte) {
  printk((pte & PTE_R) ? "R" : "_");
  printk((pte & PTE_W) ? "W" : "_");
  printk((pte & PTE_X) ? "X" : "_");
  printk((pte & PTE_U) ? "U" : "_");
  printk((pte & PTE_G) ? "G" : "_");
  printk((pte & PTE_A) ? "A" : "_");
  printk((pte & PTE_D) ? "D" : "_");
}

void
print_pgtbl_level(pagetable_t pagetable, int level)
{
  for (int i = 0; i < 512; i++) {
    pte_t pte = pagetable[i];
    if (pte & PTE_V) {
      uint64 child_pa = PTE2PA(pte);
      for(int j = 0; j < (2 - level); j++) printk(".........");
      printk("0x%x -> 0x%lx ", i, child_pa);
      print_pte_flags(pte);
      printk("\n");
      if (level > 0) {
        print_pgtbl_level((pagetable_t)child_pa, level - 1);
      }
    }
  }
}


uint64
sys_pages(void)
{
  struct proc *p = myproc();
  printk("PAGETABLE %p\n", p->pagetable);
  print_pgtbl_level(p->pagetable, 2);
  return 0;
}
