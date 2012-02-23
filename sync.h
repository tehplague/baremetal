/*
 * =====================================================================================
 *
 *       Filename:  sync.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 16:11:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef SYNC_H
#define SYNC_H

#include "types.h"

typedef volatile int mutex_t;
#define MUTEX_INITIALIZER           ((mutex_t)1)
#define MUTEX_INITIALIZER_LOCKED    ((mutex_t)0)
void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);
int mutex_trylock(mutex_t *m);

typedef struct {
    volatile unsigned cnt;
    volatile unsigned epoch;
    volatile unsigned max;
} barrier_t;
//#define BARRIER_INITIALIZER(m)  (barrier_t){ .cnt=0, .epoch=0, .max=m };
#define BARRIER_INITIALIZER(m)  { .cnt=0, .epoch=0, .max=m };
void barrier_init(barrier_t *b, int max);
void barrier(barrier_t *b);

typedef struct {
    volatile unsigned flag;
    volatile unsigned next;
} flag_t;
//#define FLAG_INITIALIZER  (flag_t){ .flag=0, .next=0 };
#define FLAG_INITIALIZER  { .flag=0, .next=0 };
void flag_init(flag_t *flag);
void flag_signal(flag_t *flag);
void flag_wait(flag_t *flag);
int flag_trywait(flag_t *flag);

unsigned collective_only(cpumask_t mask);
void collective_end();

#endif  // SYNC_H

