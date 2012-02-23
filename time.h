/*
 * =====================================================================================
 *
 *       Filename:  time.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 13:53:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef TIME_H
#define TIME_H

void udelay(unsigned long us);

inline static uint64_t rdtsc(void)
{
	union {
		uint64_t u64;
		uint32_t u32[2];
	} x;
	__asm__ volatile ("lfence\n\t rdtsc\n\t lfence" : "=a" (x.u32[0]), "=d"(x.u32[1]));
	return x.u64;
}

#endif // TIME_H
