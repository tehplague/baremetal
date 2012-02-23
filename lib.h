/*
 * =====================================================================================
 *
 *       Filename:  lib.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12.09.2011 13:43:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef LIB_H
#define LIB_H

// note: var and bit are evaluated only once!
#define BIT_SET(var, bit)       ((var) |=  (1 << (bit)))
#define BIT_CLEAR(var, bit)     ((var) &= ~(1 << (bit)))
#define MASK_SET(var, mask)       ((var) |=  (mask))
#define MASK_CLEAR(var, mask)     ((var) &= ~(mask))
#define IS_BIT_SET(var, bit)    (((var) & (1 << bit)) != 0)
#define IS_BIT_CLEAR(var, bit)  (((var) & (1 << bit)) == 0)
#define IS_MASK_SET(var, mask)    (((var) & (mask)) != 0)
#define IS_MASK_CLEAR(var, mask)  (((var) & (mask)) == 0)

/*
 * MASK(n) - create bit-mask of n 1s
 * Example: MASK(3) returns 0b0111 = 0x07
 *          MASK(8) returns 0x1111_1111 = 0xFF
 * Attention: Overflow, don't use with 32 (on x86_32) or 64 (on x86_64)
 *
 */
#define MASK(n)  ((1<<(n))-1)

/*
 * BITS_FROM_CNT(reg, from, cnt) - extracts cnt bits from reg(ister) beginning at position from 
 * BITS_FROM_TO(reg, from, to) - extracts the bits from reg(ister) from:to (from<to, both including)
 * Example: BITS_FROM_TO(0x12345678, 8, 15)  -> 0x56
 *          BITS_FROM_CNT(0xdeadbeef, 8, 16) -> 0xadbe
 */
#define BITS_FROM_CNT(reg, from, cnt)   (((reg) >> (from)) & MASK(cnt))
#define BITS_FROM_TO(reg, from, to)     BITS_FROM_CNT((reg), (from), ((to)-(from)+1))


void *memcpy(void *dest, const void *src, int count);
void *memset(void *dest, int val, int count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);
int strlen(const char *str);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
int atoi(const char *a);

int abs(int j);
long labs(long j);

#if ! __x86_64__
uint64_t __udivdi3(uint64_t n, uint64_t d);
#endif

#endif // LIB_H
