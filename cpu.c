/*
 * =====================================================================================
 *
 *       Filename:  cpu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:04:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "system.h"
#include "sync.h"
#include "smp.h"
#include "smm.h"
#include "config.h"
#include "cpu.h"

#define IFV   if (VERBOSE > 0)
#define IFVV  if (VERBOSE > 1)

#define KBRD_INTRFC  0x64
#define KBRD_BIT_KDATA 0
#define KBRD_BIT_UDATA 1

#define KBRD_IO  0x60
#define KBRD_RESET 0xFE

#define bit(n)   (1 << (n))
#define check_flag(flags, n)  ((flags) & bit(n))

extern volatile unsigned cpu_online;

char *vendor_string[] = {"unknown", "Intel", "AMD"};

void reboot()
{
    int s= 9 + cpu_online, i;
    char msg[] = "Reboot in                  ";
    char *m = &msg[0];
    
    while (*m != 0) status_putch(s++, *m++);
    s = 9 + cpu_online + 10;

    for (i=1; i<=5; i++) {
        status_putch(s++, '6'-i);
        udelay(1000*1000);
    }
    status_putch(s++, '0');
    //udelay(10);

#if 1
    char temp;
    __asm__ volatile ("CLI");
    /* empty keyboard buffer */
    do {
        temp = inportb(KBRD_INTRFC);
        if (check_flag(temp, KBRD_BIT_KDATA) != 0) {
            inportb(KBRD_IO);
        }
    } while (check_flag(temp, KBRD_BIT_UDATA) != 0);

    /* issue reboot command */
    outportb(KBRD_INTRFC, KBRD_RESET);

    udelay(1000*1000);
    status_putch(s++, '+');
#endif

#if 1
    static struct {
        unsigned short length;
        unsigned long base;
    } __attribute__((__packed__)) IDTR;
 
    IDTR.length = 0;
    IDTR.base = (unsigned long)0;
    __asm__( "lidt %0" : : "m"(IDTR) );
    __asm__ volatile ("int $32");

    udelay(1000*1000);
    status_putch(s++, '+');
#endif

    //udelay(100);
    smp_status(STATUS_STOP);

    while (1) __asm__ volatile ("hlt");
}
void stop()
{
    static unsigned cpus_halted = 0;
    static mutex_t m = MUTEX_INITIALIZER;

    mutex_lock(&m);
    cpus_halted++;
    mutex_unlock(&m);

    IFV printf("halt CPU %d (now %d down)\n", my_cpu_info()->cpu_id, cpus_halted);
    smp_status(STATUS_STOP);
    if (cpus_halted < cpu_online) {
        while (1) __asm__ volatile ("hlt");
    } else {
#       if SCROLLBACK_BUF_SIZE
            smm_restore();
            video_scrollback();
#       endif
        reboot();
    }

}
		
