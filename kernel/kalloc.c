// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

// physical page reference count, index start from KERNBASE
// a page: 4k = 2**12
extern char pa_ref_cnt[];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    // when first free physical page,
    // pa_ref_cnt should set to 1 so it will be 0 after free
    pa_ref_cnt[((uint64)p-KERNBASE)>>12] = 1;
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  char cnt = pa_ref_cnt[((uint64)pa-KERNBASE)>>12];
  if (cnt == 0)
    panic("free physical page with 0 reference");
  if (cnt == 1) {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  pa_ref_cnt[((uint64)pa-KERNBASE)>>12] -= 1;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    pa_ref_cnt[((uint64)r-KERNBASE)>>12] = 1;
  }
  return (void*)r;
}

// if want to write a page with PTE_C
int cow_process(pte_t *pte) {
  uint64 pa;
  uint flags;
  char cnt;

  struct proc *p = myproc();
  pa = PTE2PA(*pte);
  flags = PTE_FLAGS(*pte);
  cnt = pa_ref_cnt[((uint64)pa-KERNBASE)>>12];

  if ((flags&PTE_C) == 0)
    panic("COW error: don't have PTE_C");

  if (cnt>1) {
    flags &= (~PTE_C);
    flags |= PTE_W;
    char *mem;
    mem = kalloc();
    if (mem == 0) {
      printf("COW error: no free memory for cow\n");
      setkilled(p);
      return -1;
    }
    memmove(mem, (char*)pa, PGSIZE);
    *pte = PA2PTE(mem) | flags;
    pa_ref_cnt[((uint64)pa-KERNBASE)>>12] -= 1;
  } else if (cnt==1) {
    *pte |= PTE_W;
  } else {
    panic("COW error: have PTE_C while cnt==0");
  }
  *pte &= (~PTE_C);
  return 0;
}
