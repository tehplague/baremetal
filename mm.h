/*
 * =====================================================================================
 *
 *       Filename:  mm.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21.10.2011 10:27:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef MM_H
#define MM_H

typedef     unsigned long    frame_t;    // the number of a physical page frame
typedef     unsigned long    page_t;     // the number of a virtual page

int mm_init();

#define MM_WRITE_THROUGH    0x0010
#define MM_CACHE_DISABLE    0x0020

void *heap_alloc(unsigned nbr_pages, unsigned flags) __attribute__ ((malloc));
void heap_reconfig(void *p, size_t size, unsigned flags);
void tlb_shootdown(void *adr, size_t size);



#endif // MM_H

