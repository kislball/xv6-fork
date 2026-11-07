#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

#define NBINS        8
#define MINBLOCK     64
#define MAXSMALL     4096
#define MAXBLOCKSIZE (MINBLOCK << (NBINS - 1))
#define FREE         (~0U)

struct slab;
typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
    uint requested;
    struct slab *slab;
  } s;
  Align x;
};

typedef union header Header;

struct slab {
  struct slab *next;
  uint bytes;
  uint live;
};

static Header *bins[NBINS];
static struct slab *slabs;
static Header base;
static Header *freep;
static Header *livebig;
static uint64 reserved;

static void
freebig(void *ap)
{
  Header *bp, *p;

  bp = (Header *)ap - 1;
  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if (bp + bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header *
morecore(uint nu)
{
  char *p;
  Header *hp;

  if (nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if (p == SBRK_ERROR)
    return 0;
  reserved += nu * sizeof(Header);
  hp = (Header *)p;
  hp->s.size = nu;
  freebig((void *)(hp + 1));
  return freep;
}

static void *
mallocbig(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  if ((prevp = freep) == 0) {
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      p->s.slab = 0;
      p->s.requested = nbytes;
      p->s.ptr = livebig;
      livebig = p;
      return (void *)(p + 1);
    }
    if (p == freep)
      if ((p = morecore(nunits)) == 0)
        return 0;
  }
}

static uint
blockbin(uint size)
{
  uint bin = 0;
  while (bin + 1 < NBINS && (MINBLOCK << (bin + 1)) <= size)
    bin++;
  return bin;
}

static void
push(Header *p)
{
  Header **link = &bins[blockbin(p->s.size)];
  while (*link && (uint64)*link < (uint64)p)
    link = &(*link)->s.ptr;
  p->s.requested = FREE;
  p->s.ptr = *link;
  *link = p;
}

static void
removeblock(Header *p)
{
  Header **link = &bins[blockbin(p->s.size)];
  while (*link != p)
    link = &(*link)->s.ptr;
  *link = p->s.ptr;
}

static int
newslab(void)
{
  uint bytes = PGROUNDUP(sizeof(struct slab) + MAXBLOCKSIZE);
  struct slab *s = (struct slab *)sbrk(bytes);
  if ((char *)s == SBRK_ERROR)
    return -1;
  reserved += bytes;
  s->bytes = bytes;
  s->live = 0;
  s->next = slabs;
  slabs = s;
  Header *p = (Header *)(s + 1);
  p->s.slab = s;
  p->s.size = MAXBLOCKSIZE;
  push(p);
  return 0;
}

static void
reclaim(void)
{
  for (;;) {
    struct slab *s = slabs;
    if (s == 0 || s->live != 0 || (char *)s + s->bytes != sbrk(0))
      return;
    char *end = (char *)(s + 1) + MAXBLOCKSIZE;
    for (Header *p = (Header *)(s + 1); (char *)p < end;
         p = (Header *)((char *)p + p->s.size))
      removeblock(p);
    struct slab *next = s->next;
    uint bytes = s->bytes;
    if (sbrk(-(int)bytes) == SBRK_ERROR) {
      for (Header *p = (Header *)(s + 1); (char *)p < end;
           p = (Header *)((char *)p + p->s.size))
        push(p);
      return;
    }
    reserved -= bytes;
    slabs = next;
  }
}

static void *
mallocsmall(uint nbytes)
{
  uint size = (nbytes + sizeof(Header) + MINBLOCK - 1) / MINBLOCK * MINBLOCK;
  Header *p = 0;
  for (uint bin = blockbin(size); bin < NBINS; bin++) {
    for (p = bins[bin]; p; p = p->s.ptr)
      if (p->s.size >= size)
        break;
    if (p)
      break;
  }
  if (p == 0) {
    if (newslab() < 0)
      return 0;
    p = (Header *)(slabs + 1);
  }
  removeblock(p);
  if (p->s.size - size >= sizeof(Header) + 1) {
    Header *rest = (Header *)((char *)p + size);
    rest->s.size = p->s.size - size;
    rest->s.slab = p->s.slab;
    push(rest);
    p->s.size = size;
  }
  p->s.requested = nbytes;
  p->s.slab->live++;
  return p + 1;
}

static void
freesmall(Header *p)
{
  struct slab *s = p->s.slab;
  uint limit = blockbin(p->s.size) + 1;
  Header *next = (Header *)((char *)p + p->s.size);
  char *end = (char *)(s + 1) + MAXBLOCKSIZE;
  s->live--;
  if ((char *)next < end && next->s.requested == FREE &&
      blockbin(p->s.size + next->s.size) <= limit) {
    removeblock(next);
    p->s.size += next->s.size;
  }
  push(p);
  reclaim();
}

static void
dumpheap(void)
{
  pages();
  uint64 used = 0;
  for (struct slab *s = slabs; s; s = s->next) {
    char *end = (char *)(s + 1) + MAXBLOCKSIZE;
    for (Header *p = (Header *)(s + 1); (char *)p < end;
         p = (Header *)((char *)p + p->s.size))
      if (p->s.requested != FREE)
        used += p->s.requested;
  }
  for (Header *p = livebig; p; p = p->s.ptr)
    used += p->s.requested;
  fprintf(2, "heap: brk=%p used=%lu reserved=%lu utilization=%lu%%\n", sbrk(0),
          used, reserved, reserved ? used * 100 / reserved : 0);
  for (struct slab *s = slabs; s; s = s->next) {
    fprintf(2, " slab=%p bytes=%u live=%u next=%p\n", s, s->bytes, s->live,
            s->next);
    char *end = (char *)(s + 1) + MAXBLOCKSIZE;
    for (Header *p = (Header *)(s + 1); (char *)p < end;
         p = (Header *)((char *)p + p->s.size))
      fprintf(2, "  small=%p capacity=%lu %s requested=%u\n", p + 1,
              p->s.size - sizeof(Header),
              p->s.requested == FREE ? "free" : "used",
              p->s.requested == FREE ? 0 : p->s.requested);
  }
  for (uint bin = 0; bin < NBINS; bin++) {
    uint upper = bin == NBINS - 1 ? MAXBLOCKSIZE : (MINBLOCK << (bin + 1)) - 1;
    fprintf(2, " bin[%u] range=%u..%u:", bin, MINBLOCK << bin, upper);
    for (Header *p = bins[bin]; p; p = p->s.ptr)
      fprintf(2, " %p", p + 1);
    fprintf(2, "\n");
  }
  for (Header *p = livebig; p; p = p->s.ptr)
    fprintf(2, " large=%p bytes=%lu used requested=%u\n", p + 1,
            p->s.size * sizeof(Header), p->s.requested);
  if (freep) {
    Header *p = freep;
    do {
      if (p != &base)
        fprintf(2, " large=%p bytes=%lu free next=%p\n", p + 1,
                p->s.size * sizeof(Header), p->s.ptr);
      p = p->s.ptr;
    } while (p != freep);
  }
}

#define ALWAYS_BIG 0

void *
malloc(uint nbytes)
{
  if (nbytes > 0x7fffffffU - 2 * sizeof(Header))
    return 0;
  void *p =
    nbytes > MAXSMALL || ALWAYS_BIG ? mallocbig(nbytes) : mallocsmall(nbytes);
  if (p)
    dumpheap();
  return p;
}

void
free(void *ap)
{
  if (ap == 0)
    return;
  Header *p = (Header *)ap - 1;
  if (p->s.slab && !ALWAYS_BIG) {
    freesmall(p);
  } else {
    Header **link = &livebig;
    while (*link != p)
      link = &(*link)->s.ptr;
    *link = p->s.ptr;
    freebig(ap);
  }
  dumpheap();
}