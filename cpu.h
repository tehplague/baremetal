/*
 * =====================================================================================
 *
 *       Filename:  cpu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:05:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef CPU_H
#define CPU_H

#include "system.h"

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
static inline unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}
static inline uint32_t inportl (unsigned short _port)
{
    uint32_t rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
static inline void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
static inline void outportl (unsigned short _port, uint32_t _data)
{
    __asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

static inline void halt()
{
    printf("System halted.");
    while (1) {
        __asm__ volatile ("hlt");
    }
}



/*
 * rdmsr(), wrmsr() 
 * from perfcount.c (by Christian Spoo)
 */
static uint64_t rdmsr(uint32_t msr) {
  uint32_t low, high;

  __asm__ __volatile__ ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) | low;
}

static void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t low = value & 0xFFFFFFFF;
  uint32_t high = value >> 32;

  __asm__ __volatile__ ("wrmsr" :: "a"(low), "c"(msr), "d"(high));
}





void reboot();
void stop();

inline static unsigned sti(void) {
    ptr_t flags;
    __asm__ volatile ("pushf; sti; pop %0" : "=r"(flags) : : "memory");
    return (flags & (1<<9));
}

/* return if IF was previously set (to use sti afterwards conditionally) */
inline static unsigned cli(void) {
    ptr_t flags;
    __asm__ volatile ("pushf; cli; pop %0" : "=r"(flags) : : "memory");
    return (flags & (1<<9));
}

inline static void cpuid(uint32_t func, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func));
}

inline static void cpuid2(uint32_t func, uint32_t subfunc, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func), "c"(subfunc));
}

inline static uint32_t cpuid_eax(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return eax;
}

inline static uint32_t cpuid_ebx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return ebx;
}

inline static uint32_t cpuid_ecx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return ecx;
}

inline static uint32_t cpuid_edx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return edx;
}


#endif //  CPU_H
