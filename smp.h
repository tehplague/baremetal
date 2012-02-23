/*
 * =====================================================================================
 *
 *       Filename:  smp.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 13:23:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef SMP_H
#define SMP_H

#include "system.h"
#include "config.h"
#include "sync.h"

/*
 * per-cpu info structure
 */
#define SMP_FLAG_HALT   (1u <<0)
#define SMP_FLAG_HALTED (1u <<1)
typedef struct {
    unsigned cpu_id;   
    volatile unsigned flags;
    mutex_t wakelock;
} cpu_info_t;


/*
 * Stack (growing downwards with per-cpu info structure on opposite (lower) end, where the stack should never grow to)
 */
typedef union {
    unsigned stack[STACK_FRAMES * 4096 / sizeof(unsigned)];
    cpu_info_t info;
} stack_t; // __attribute__((packed));

extern stack_t stack[MAX_CPU];

int smp_init(void);

static inline volatile cpu_info_t * my_cpu_info()
{
    void volatile * p;
#   ifdef __x86_64__
    __asm__ volatile("movq %%rsp, %%rax \n\t andq %%rdx, %%rax" : "=a"(p) : "d" ( ~STACK_MASK ));
#   else
    __asm__ volatile("movl %%esp, %%eax \n\t andl %%edx, %%eax" : "=a"(p) : "d" ( ~STACK_MASK ));
#   endif
    return (cpu_info_t volatile * )p;
}

#define STATUS_WAKEUP   '^'
#define STATUS_RUNNING  '.'
#define STATUS_MUTEX    'm'
#define STATUS_BARRIER  'b'
#define STATUS_FLAG     'f'
#define STATUS_DELAY    'd'
#define STATUS_HALT     '-'
#define STATUS_ERROR    'E'
#define STATUS_STOP     '_'
#define STATUS_NOTUP    '|'

void smp_status(char c);

void smp_halt(void);
void smp_wakeup(unsigned cpu_id);

#endif // SMP_H

