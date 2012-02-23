
Memory Management
=================

The header mm.h is for both 32 and 64 bit kernels. In mm.c, there are
#if/#else/#endif blocks where a distinction between 32 and 64 bit is needed.
All structure definitions are in mm_struct.h

Some Numbers:
                                    32 bit      64 bit
size of a frame:                    4 kB        4 kB
size of an entry in page table:     4 B         8 B
number of entries in page table:    1024        512
size of all frame is sub-level      4 MB        2 MB
   -"-   for sub-sub-level:         4 GB        1 GB


needed bit-field for available pages (limited to 4 GB usable memory):
    1 M bits = 128 kByte
(currently limited to 2GB)


Memory Layout
-------------
   0 .. 1 MB    low memory (boot code, first page tables, parts reserved)
   1 .. 2 MB    kernel
   2 .. 4 MB    reserved for further growth of kernel
     >4 MB      Heap (free for page tables and malloc)

Notation
--------
frame : number of physical page frame, 0..N
offset : offset of physical page frame, linear address, (frame << 12)
page : number of virtual page
adr(ess) : virtual address/paging, (page << 12)
