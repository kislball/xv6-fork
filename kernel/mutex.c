#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

struct mutex {
  struct sleeplock lock;
};

int
mutexalloc(struct file **f) {
  struct mutex *mux;
  struct proc *p = myproc();
  mux = 0;
  *f = 0;

  mux = kalloc();
  if (mux == 0) {
    printk("mutexalloc: kalloc failed for pid=%d\n", p->pid);
    goto bad;
  }
  initsleeplock(&mux->lock, "mutex");

  *f = filealloc();
  if (*f == 0) {
    printk("mutexalloc: filealloc failed for pid=%d\n", p->pid);
    goto bad;
  } 
  (*f)->mutex = mux;
  (*f)->type = FD_MUTEX;
  (*f)->readable = 0;
  (*f)->writable = 0;

  printk("mutexalloc: kalloc allocated mutex=%p for pid=%d\n", mux, p->pid);
  return 0;
bad:
  if (mux) {
    printk("mutexalloc: kfree mutex=%p for pid=%d after failure\n", mux, p->pid);
    kfree(mux);
  }
  if (*f) fileclose(*f);
  printk("mutexalloc: failed for pid=%d\n", p->pid);
  return -1;
}

void
mutexclose(struct mutex *mux) {
  struct proc *p = myproc();
  printk("mutexclose: freeing mutex=%p for pid=%d\n", mux, p->pid);
  kfree(mux);
}

int
mutexrelease(struct mutex *mux) {
  struct proc *p = myproc();
  if (holdingsleep(&mux->lock)) {
    printk("mutexrelease: releasing mutex=%p for pid=%d\n", mux, p->pid);
    releasesleep(&mux->lock);
    return 0;
  } else {
    return -1;
  }
}

void
mutexlock(struct mutex *mux) {
  struct proc *p = myproc();
  acquiresleep(&mux->lock);
  printk("mutexlock: acquired mutex=%p for pid=%d\n", mux, p->pid);
}
