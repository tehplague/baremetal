/*
 * =====================================================================================
 *
 *       Filename:  perfcount.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.02.2012 10:02:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Christian Spoo (cs) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef PERFCOUNT_H
#define PERFCOUNT_H

#include "stddef.h"

#define PERFCOUNT_L1DATA    (uint64_t)0xFF000151ull
#define PERFCOUNT_L2        (uint64_t)0xFF000224ull
#define PERFCOUNT_L3        (uint64_t)0xFF000309ull

#define PERFCOUNT_CORE_PMC(n)   (n & 0x7F)
#define PERFCOUNT_UNCORE_PMC(n) (n | 0x80)

void perfcount_init(unsigned int counter, uint64_t config);
uint64_t perfcount_raw(uint8_t event, uint8_t umask);

void perfcount_start(uint8_t counter);
void perfcount_stop(uint8_t counter);
void perfcount_reset(uint8_t counter);
uint64_t perfcount_read(uint8_t counter);

#endif

