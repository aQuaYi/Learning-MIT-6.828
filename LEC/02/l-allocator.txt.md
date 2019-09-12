# 6.S081/6.828 2019 Lecture 2: Memory allocation

Systems programming
  Build services for applications
    Not specific to an application workload
  Use underlying OS directly
  Examples:
    unix utilities, K/V servers, ssh, bigint library, etc.

Challenges:
  Low-level programming environment
    E.g., numbers are 64 bits (not unlimited integers)
    E.g., allocating/freeing memory  (not typed objects)
  Concurrency
    to allow requests to be processed in parallel
  Crashes
  Performance
    Deliver hardware performance to applications

Dynamic memory allocation is a basic system service
  Example of low-level programming
    Pointer manipulation, type casting, etc.
    Use underlying OS to ask for chunks of memory
  Must support wide-range of workloads
  Performance is important
    When you build an application, memory allocator can be a bottleneck
    Likely you will look at memory allocators at some point in your career

Background: program layout
  text
  data (static storage)
  stack (stack storage)
  heap (dynamic memory allocation)
    Grow heap with sbrk() or mmap()
  [text |  data | heap ->  ...    <- stack]
  0                                       top of address space

Memory allocated in data segment
  static, always there

Memory allocated on stack
  allocated on function entry
  freed on function exit

Memory allocator for heap
  interface
    p = malloc(int sz)
    free(p)
  several implementations in this lecture
    all single threaded

Goals for heap allocator
  Fast malloc and free
  Small memory overhead
  Want to use all memory
    Avoid fragmentation

Simple implementation (e.g., xv6's kalloc)
  present heap as a list of blocks, sorted by address
  200 bytes heap
    [ free     ]
    0         200
  malloc(100) -> 0
  malloc(50) -> 100
  heap picture:
    [ A    |   B  | free ]
    0     100    150    200

API challenge
  free(100)
    what is size for B?
  need to store size for B somewhere
  put a header front of each block with some metadata
  K&R: struct header: contains size and next pointer
    [ H   A   |  H  free  | H free ]
    0  16   100+16     150+32     200
  (Assume: 64-bit pointers and sizes)

Fragmentation challenge
  malloc(75) -> fails!
    no block on free list is large enough
  we need to coalesce

K&R malloc (see user/umalloc.c, used by xv6's shell)
  first-fit on malloc
    other policies: best-fit
  coalesce on free
  memory overhead: if every malloc is for 16 bytes, 50%
  efficiency:
    workload dependent
    e.g., if blocks are returned in order: free list has one element
    e.g., if blocks are returned out of order: perhaps search long list

Aside: C
  Popular for systems programming language
    Many kernels are written in C (e.g., Linux, BSDs, MacOS, Windows)
    Much system software written in C (K/V servers, ssh, etc.)
  Allows stepping out of type system
    A piece of memory can be stated to be of type T1
    Same memory may also used as type T2
    Type casts (Header *)
  Pointers = addresses
    Allows referencing memory, devices, etc.
  Examples in K&R malloc:
    char *p (pointer to an char object)
    Header *p; where does p+1 point to?
    void *p (any type of pointer)
  Dangerous programming language
    Easy to have bugs
      No type safety
      Not great for concurrency
  Next lecture: TA lecture on C and gdb

Region allocator (see malloc.c)
  faster allocator, but fragmentation
  region for part of the heap
    very fast malloc
    very fast free
    low memory overhead
  serious fragmentation
    frees region until complete region is unused
  application must fit region use
  more efficient than K&R and still general-purpose?

Buddy allocator (see malloc.c)
  topic of one of the labs
  data structure:
    size classes: 2^0, 2^1, 2^2, ... 2^k
      size 0: minimum size (e.g., 16 bytes)
        # blocks = 2^k * min size
	min size should be large enough to keep two pointers for free list
      size 1: blocks of 2^1
        two buddies at level 0 are merged into 1 size 1 block
      ... and so on ...
      size k: the complete heap
        # blocks: 1
    free list per size
      blocks that are free of that size
    bit vectors for each block at each size
      alloc: is block allocated?
      split: is block split?
    [draw diagram]
  internal fragmentation
    malloc(17) allocates 32 bytes
  operations:
    malloc: allocate first free block on list
      perhaps after splitting a bigger block
    free:
      determine size of block
      if buddy is free, merge
      put block on free list of size class
  why good?
    malloc/free are fast
    cost: size of data structures
      several optimizations possible

Other sequential allocators:
  dlmalloc
  slab allocator

Other goals
  small memory overhead
    metadata for example buddy allocator is large
  good memory locality
  good scalability with increasing cores
    concurrent malloc/free

References

- <https://en.wikipedia.org/wiki/Buddy_memory_allocation>
- <http://gee.cs.oswego.edu/dl/html/malloc.html>
- <http://www.usenix.org/publications/library/proceedings/bos94/full_papers/bonwick.ps>
- <http://people.freebsd.org/~jasone/jemalloc/bsdcan2006/jemalloc.pdf>
- <http://www.cs.umass.edu/~emery/pubs/berger-asplos2000.pdf>
- <http://bitsquid.blogspot.com/2015/08/allocation-adventures-3-buddy-allocator.html>

Knuth, Donald (1997). Fundamental Algorithms. The Art of Computer
Programming. 1 (Second ed.). Reading, Massachusetts:
Addison-Wesley. pp. 435鈥�455. ISBN 0-201-89683-4.
