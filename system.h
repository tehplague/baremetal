#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stddef.h>
#include "config.h"
#include "info.h"
#include "time.h"

/* Verbosity: if not defined here, modules may define their own */
//#define VERBOSE 0

#define PAGE_BITS    12
#define PAGE_SIZE    (1<<PAGE_BITS)
#define PAGE_MASK    (PAGE_SIZE-1)
#if __x86_64__
#   define INDEX_BITS   9
#else
#   define INDEX_BITS  10
#endif
#define INDEX_MASK    ((1<<INDEX_BITS) -1)

#define STACK_SIZE    ((ptr_t)STACK_FRAMES * PAGE_SIZE)
#define STACK_MASK    (STACK_SIZE-1)

#if __x86_64__
struct regs
{
	unsigned long long cr2, cr3, gs, fs, es, ds;				/* pushed the segs last */
	unsigned long long r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, _zero, rbx, rdx, rcx, rax ;
	unsigned long long int_no, err_code;			/* our 'push byte #' and ecodes do this */
	unsigned long long rip, cs, rflags, rsp, ss;			/* pushed by the processor automatically */
} __attribute__((packed));
#else
struct regs
{
    unsigned int cr2, cr3, gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cx, eflags, useresp, ss;
};
#endif


/*
 * int i;
 * int array[] = {1,2,3,4};
 * foreach(i, array) {
 *   printf("%i\n", i);
 * }
 */
#define foreach(item, array) \
        for(int keep = 1, \
                count = 0,\
                size = sizeof (array) / sizeof *(array); \
                keep && count != size; \
                keep = !keep, count++) \
        for(item = array[count]; keep; keep = !keep)
//#define foreach(item, array) \-
//    for (int count=0, size = sizeof(array)/sizeof *(array), item=array[count]; count < size; count++)




/* lib.c */
#include "lib.h"

#define NULL ((void*)0)

/* idt.c */
void idt_install();
void idt_install_ap();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/* isr.c */
void isr_install();

/* apic.c */
#include "apic.h"

/* scrn.c */
void cls();
void putch(char c);
void status_putch(int x, int c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
#if SCROLLBACK_BUF_SIZE
void init_video_scrollback(void);
void video_scrollback(void);
#endif
void itoa (char *buf, int base, long d);
void printf (const char *format, ...);
    //// __attribute__ (( format(printf, 1, 2) ));  <-- this is a bad idea, simply because we're not really compatible...


#endif
