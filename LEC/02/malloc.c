#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/mman.h>

#include <stdint.h>

// Three simple allocators: K&R, region, and buddy
// compile on Linux: $ gcc -O2 -o malloc malloc.c
// run: $ ./malloc

//
// K&R allocator (Section 8.7)
//

#define NALLOC  1024  /* minimum #units to request */

struct header {
  struct header *ptr;
  size_t size;
};

typedef struct header Header;

static Header base;
static Header *freep = NULL;

void kr_free(void *p);

static Header *moreheap(size_t nu)
{
  char *cp;
  Header *up;

  if (nu < NALLOC)
    nu = NALLOC;
  cp = sbrk(nu * sizeof(Header));
  if (cp == (char *) -1)
    return NULL;
  up = (Header *) cp;
  up->size = nu;
  kr_free((void *)(up + 1));
  return freep;
}

/* malloc: general-purpose storage allocator */
void *
kr_malloc(size_t nbytes)
{
  Header *p, *prevp;
  unsigned nunits;

  nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  if ((prevp = freep) == NULL) {	/* no free list yet */
    base.ptr = freep = prevp = &base;
    base.size = 0;
  }
  for (p = prevp->ptr; ; prevp = p, p = p->ptr) {
    if (p->size >= nunits) {	/* big enough */
      if (p->size == nunits)	/* exactly */
	prevp->ptr = p->ptr;
      else {	/* allocate tail end */
	p->size -= nunits;
	p += p->size;
	p->size = nunits;
      }
      freep = prevp;
      return (void *) (p + 1);
    }
    if (p == freep) {	/* wrapped around free list */
      if ((p = (Header *) moreheap(nunits)) == NULL) {
	return NULL;	/* none left */
      }
    }
  }
}

/* free: put block ap in free list */
void
kr_free(void *ap)
{
  Header *bp, *p;

  if (ap == NULL)
    return;

  bp = (Header *) ap - 1;	/* point to block header */
  for (p = freep; !(bp > p && bp < p->ptr); p = p->ptr)
    if (p >= p->ptr && (bp > p || bp < p->ptr))
      break;	/* freed block at start or end of arena */

  if (bp + bp->size == p->ptr) {	/* join to upper nbr */
    bp->size += p->ptr->size;
    bp->ptr = p->ptr->ptr;
  } else {
    bp->ptr = p->ptr;
  }

  if (p + p->size == bp) {	/* join to lower nbr */
    p->size += bp->size;
    p->ptr = bp->ptr;
  } else {
    p->ptr = bp;
  }

  freep = p;
}

void
kr_print(void) {
  Header *p, *prevp = freep;

 for (p = prevp->ptr; ; prevp = p, p = p->ptr) {
   printf("%p %d\n", p, p->size);
   if (p == freep)
     break;
 }
}

//
// Region-based allocator, a special-purpose allocator
//

#define HEAPINC (100*4096)

struct region {
  void *start;
  void *cur;
  void *end;
};
typedef struct region Region;

static Region rg_base;

Region *
rg_create(size_t nbytes)
{
  rg_base.start = sbrk(nbytes);
  rg_base.cur = rg_base.start;
  rg_base.end = rg_base.start + HEAPINC;
  return &rg_base;
}

void *
rg_malloc(Region *r, size_t nbytes)
{
  assert (r->cur + nbytes <= r->end);
  void *p = r->cur;
  r->cur += nbytes;
  return p;
}

// free all memory in region resetting cur to start
void
rg_free(Region *r) {
  r->cur = r->start;
}

//
// Buddy allocator
//

#define LEAF_SIZE     16 // The smallest allocation size (in bytes)
#define NSIZES        15 // Number of entries in bd_sizes array
#define MAXSIZE       (NSIZES-1) // Largest index in bd_sizes array
#define BLK_SIZE(k)   ((1L << (k)) * LEAF_SIZE) // Size in bytes for size k
#define HEAP_SIZE     BLK_SIZE(MAXSIZE)
#define NBLK(k)       (1 << (MAXSIZE-k))  // Number of block at size k
#define ROUNDUP(n,sz) (((((n)-1)/(sz))+1)*(sz))  // Round up to the next multiple of sz

// A double-linked list for the free list of each level
struct bd_list {
  struct bd_list *next;
  struct bd_list *prev;
};
typedef struct bd_list Bd_list;

// The allocator has sz_info for each size k. Each sz_info has a free
// list, an array alloc to keep track which blocks have been
// allocated, and an split array to to keep track which blocks have
// been split.  The arrays are of type char (which is 1 byte), but the
// allocator uses 1 bit per block (thus, one char records the info of
// 8 blocks).
struct sz_info {
  struct bd_list free;
  char *alloc;
  char *split;
};
typedef struct sz_info Sz_info;

static Sz_info bd_sizes[NSIZES];
static void *bd_base;   // start address of memory managed by the buddy allocator

// List functions that the buddy allocator uses. Implementations
// are at the end of the buddy allocator code.
void lst_init(Bd_list*);
void lst_remove(Bd_list*);
void lst_push(Bd_list*, void *);
void *lst_pop(Bd_list*);
void lst_print(Bd_list*);
int lst_empty(Bd_list*);

// Return 1 if bit at position index in array is set to 1
int bit_isset(char *array, int index) {
  char b = array[index/8];
  char m = (1 << (index % 8));
  return (b & m) == m;
}

// Set bit at position index in array to 1
void bit_set(char *array, int index) {
  char b = array[index/8];
  char m = (1 << (index % 8));
  array[index/8] = (b | m);
}

// Clear bit at position index in array
void bit_clear(char *array, int index) {
  char b = array[index/8];
  char m = (1 << (index % 8));
  array[index/8] = (b & ~m);
}

void
bd_print() {
  printf("=== buddy allocator state ===\n");
  for (int k = 0; k < NSIZES; k++) {
    printf("size %d (%ld): free list: ", k, BLK_SIZE(k));
    lst_print(&bd_sizes[k].free);
    printf("  alloc:");
    for (int b = 0; b < NBLK(k); b++) {
      printf(" %d", bit_isset(bd_sizes[k].alloc, b));
    }
    printf("\n");
    if(k > 0) {
      printf("  split:");
      for (int b = 0; b < NBLK(k); b++) {
	printf(" %d", bit_isset(bd_sizes[k].split, b));
      }
      printf("\n");
    }
  }
}

// Allocate memory for the heap managed by the allocator, and allocate
// memory for the data structures of the allocator.
void
bd_init() {
  bd_base = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (bd_base == MAP_FAILED) {
    fprintf(stderr, "couldn't map heap; %s\n", strerror(errno));
    assert(bd_base);
  }
  // printf("bd: heap size %d\n", HEAP_SIZE);
  for (int k = 0; k < NSIZES; k++) {
    lst_init(&bd_sizes[k].free);
    int sz = sizeof(char)*ROUNDUP(NBLK(k), 8)/8;
    bd_sizes[k].alloc = malloc(sz);
    memset(bd_sizes[k].alloc, 0, sz);
  }
  for (int k = 1; k < NSIZES; k++) {
    int sz = sizeof(char)*ROUNDUP(NBLK(k), 8)/8;
    bd_sizes[k].split = malloc(sz);
    memset(bd_sizes[k].split, 0, sz);
  }
  lst_push(&bd_sizes[MAXSIZE].free, bd_base);
}

// What is the first k such that 2^k >= n?
int
firstk(size_t n) {
  int k = 0;
  size_t size = LEAF_SIZE;

  while (size < n) {
    k++;
    size *= 2;
  }
  return k;
}

// Compute the block index for address p at size k
int
blk_index(int k, char *p) {
  int n = p - (char *) bd_base;
  return n / BLK_SIZE(k);
}

// Convert a block index at size k back into an address
void *addr(int k, int bi) {
  int n = bi * BLK_SIZE(k);
  return (char *) bd_base + n;
}

void *
bd_malloc(size_t nbytes)
{
  int fk, k;

  assert(bd_base != NULL);

  // Find a free block >= nbytes, starting with smallest k possible
  fk = firstk(nbytes);
  for (k = fk; k < NSIZES; k++) {
    if(!lst_empty(&bd_sizes[k].free))
      break;
  }
  if(k >= NSIZES)  // No free blocks?
    return NULL;

  // Found one; pop it and potentially split it.
  char *p = lst_pop(&bd_sizes[k].free);
  bit_set(bd_sizes[k].alloc, blk_index(k, p));
  for(; k > fk; k--) {
    char *q = p + BLK_SIZE(k-1);
    bit_set(bd_sizes[k].split, blk_index(k, p));
    bit_set(bd_sizes[k-1].alloc, blk_index(k-1, p));
    lst_push(&bd_sizes[k-1].free, q);
  }
  // printf("malloc: %p size class %d\n", p, fk);
  return p;
}

// Find the size of the block that p points to.
int
size(char *p) {
  for (int k = 0; k < NSIZES; k++) {
    if(bit_isset(bd_sizes[k+1].split, blk_index(k+1, p))) {
      return k;
    }
  }
  return 0;
}

void
bd_free(void *p) {
  void *q;
  int k;

  for (k = size(p); k < MAXSIZE; k++) {
    int bi = blk_index(k, p);
    bit_clear(bd_sizes[k].alloc, bi);
    int buddy = (bi % 2 == 0) ? bi+1 : bi-1;
    if (bit_isset(bd_sizes[k].alloc, buddy)) {
      break;
    }
    // budy is free; merge with buddy
    q = addr(k, buddy);
    lst_remove(q);
    if(buddy % 2 == 0) {
      p = q;
    }
    bit_clear(bd_sizes[k+1].split, blk_index(k+1, p));
  }
  // printf("free %p @ %d\n", p, k);
  lst_push(&bd_sizes[k].free, p);
}

// Implementation of lists: double-linked and circular. Double-linked
// makes remove fast. Circular simplifies code, because don't have to
// check for empty list in insert and remove.

void
lst_init(Bd_list *lst)
{
  lst->next = lst;
  lst->prev = lst;
}

int
lst_empty(Bd_list *lst) {
  return lst->next == lst;
}

void
lst_remove(Bd_list *e) {
  e->prev->next = e->next;
  e->next->prev = e->prev;
}

void*
lst_pop(Bd_list *lst) {
  assert(lst->next != lst);
  Bd_list *p = lst->next;
  lst_remove(p);
  return (void *)p;
}

void
lst_push(Bd_list *lst, void *p)
{
  Bd_list *e = (Bd_list *) p;
  e->next = lst->next;
  e->prev = lst;
  lst->next->prev = p;
  lst->next = e;
}

void
lst_print(Bd_list *lst)
{
  for (Bd_list *p = lst->next; p != lst; p = p->next) {
    printf(" %p", p);
  }
  printf("\n");
}

//
// Example use case: a "weird" list of sz elements
//

struct list {
  struct list *next;
  int content;
};
typedef struct list List;

// Use malloc and free for list of sz elements
void
workload(int sz, void* malloc(size_t),  void free(void *)) {
  List *head = 0;
  List *tail;
  List *end;
  List *p, *n;
  int i = 0;

  // initialize list with 2 elements: head and tail */
  head = malloc(sizeof(List));
  tail = malloc(sizeof(List));
  head->next = tail;
  tail->next = 0;

  // every even i: insert at head; every odd i: append, so that blocks
  // aren't freed in order below
  for (i = 0; i < sz; i++) {
    p = (List *) malloc(sizeof(List));
    assert(p != NULL);
    if(i % 2 ==0) {
      p->next = head;
      head = p;
    } else {
      tail->next = p;
      p->next = 0;
      tail = p;
    }
  }

  // now free list
  for (p = head; p != NULL; p = n) {
    n = p->next;
    free(p);
  }
}

// Use a region for a list of sz elements
void
rg_workload(int sz) {
  List h;
  List *p;
  Region *r = rg_create(HEAPINC);

  h.next = NULL;
  for (int i = 0; i < sz; i++) {
    p = rg_malloc(r, 16);
    assert(p != NULL);
    p->next = h.next;
    h.next = p;
  }
  rg_free(r);
}

int
main(char *argv, int argc)
{
  enum { N = 10000 };
  struct timeval start, end;

  gettimeofday(&start, NULL);
  workload(N, kr_malloc, kr_free);
  gettimeofday(&end, NULL);
  printf("elapsed time K&R is    %d usec\n",
	 (end.tv_sec-start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);

  gettimeofday(&start, NULL);
  rg_workload(N);
  gettimeofday(&end, NULL);
  printf("elapsed time region is %d usec\n",
	 (end.tv_sec-start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);

  bd_init();
  gettimeofday(&start, NULL);
  workload(N, bd_malloc, bd_free);
  gettimeofday(&end, NULL);
  printf("elapsed time buddy is  %d usec\n",
	 (end.tv_sec-start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);

  /* set NSIZES to 4
  bd_init();
  bd_print();
  char *p1 = bd_malloc(8);
  bd_print();
  bd_free(p1);
  bd_print();
  */

  /*
  char *p2 = bd_malloc(8);
  bd_print();
  char *p3 = bd_malloc(8);
  bd_print();
  bd_free(p2);
  bd_print();
  bd_free(p3);
  bd_print();
  */
}
