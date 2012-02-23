/*
 * =====================================================================================
 *
 *       Filename:  smp.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 13:23:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "smp.h"
#include "system.h"
#include "apic.h"
#include "cpu.h"


stack_t stack[MAX_CPU] __attribute__(( aligned(STACK_SIZE) ));

int smp_init(void)
{
    /* this is run before any other CPU (AP) is called */
    unsigned u;
    for (u=0; u<MAX_CPU; u++) {
        stack[u].info.cpu_id = u;
        stack[u].info.flags = 0;
        //if (u<4) printf("stack[%u].info.cpu_id at 0x%x value: %u\n", u, &(stack[u].info.cpu_id), stack[u].info.cpu_id);
        mutex_init(&(stack[u].info.wakelock));  // state: unlocked
    }
    return 0;
}


void smp_status(char c)
{
    status_putch(6+my_cpu_info()->cpu_id, c);
}

void smp_halt(void)
{
    unsigned if_backup;
    smp_status(STATUS_HALT);
    MASK_SET(my_cpu_info()->flags, SMP_FLAG_HALT|SMP_FLAG_HALTED);
    if_backup = sti();
    while (IS_MASK_SET(my_cpu_info()->flags, SMP_FLAG_HALT)) {
        __asm__ volatile ("hlt");
    }
    MASK_CLEAR(my_cpu_info()->flags, SMP_FLAG_HALTED);
    smp_status(STATUS_RUNNING);
    if (!if_backup) cli();
}

void smp_wakeup(unsigned cpu_id)
{
    MASK_CLEAR(stack[cpu_id].info.flags, SMP_FLAG_HALT);
    udelay(5);
    while (IS_MASK_SET(stack[cpu_id].info.flags, SMP_FLAG_HALTED)) {
        send_ipi(cpu_id, 128);
        udelay(5);
    }
}

