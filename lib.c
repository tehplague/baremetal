#include "system.h"

/*
 * This file is used in 32-Bit Boot Code (REAL MODE) called from startXX.asm and boot32.c
 * as well as in the final kernel (main.c etc.)
 */

void *memcpy(void *dest, const void *src, int count)
{
    /* copy 'count' bytes of data from 'src' to 'dest', finally return 'dest' */
    char *dp = (char *)dest;
    char *sp = (char *)src;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void *memset(void *dest, int val, int count)
{
    /* set 'count' bytes in 'dest' to 'val'.  Again, return 'dest' */
    char *temp = (char *)dest;
    for( ; count != 0; count--) *temp++ = (char)val;
    return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
    /* Same as above, but this time, we're working with a 16-bit
    *  'val' and dest pointer. Your code can be an exact copy of
    *  the above, provided that your local variables if any, are
    *  unsigned short */
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

int strlen(const char *str)
{
    /* This loops through character array 'str', returning how
    *  many characters it needs to check before it finds a 0.
    *  In simple words, it returns the length in bytes of a string */
    int retval;
    for(retval = 0; *str != '\0'; str++) retval++;
    return retval;
}

int strcmp(const char *a, const char *b) 
{
    while (*a != 0 && *b != 0 && (*a == *b)) {
        a++;
        b++;
    }
    return (*a - *b);
}
int strncmp(const char *a, const char *b, int n) 
{
    while (n > 0 && *a != 0 && *b != 0 && (*a == *b)) {
        n--;
        a++;
        b++;
    }
    return n==0?0:(*a - *b);
}

/*
 * can handle decimal (not starting with 0), octal (starting with zero), hex (starting with 0x) and negative values (starting with -).
 */
int atoi(const char *a)
{
    int i = 0;
    int s = 1;
    int base = 10;
    if (*a == '-') {
        s = -1;
        a++;
    }
    if (*a == '0') {
        base = 8;
        a++;
        if (*a == 'x' || *a == 'X') {
            base = 16;
            a++;
        }
    }
    while ((*a >= '0' && *a <= '0'+(base>10?10:base)-1) || (base>10 && ((*a >= 'A' && *a <= 'A'+base-11) || (*a >= 'a' && *a <= 'a'+base-11)))) {
        i *= base;
        if (*a <= '9')
            i += *a - '0';
        else if (*a >= 'a')
            i += *a - 'a'+10;
        else
            i += *a - 'A'+10;
        a++;
    }
    i *= s;
    return i;
}


int abs(int j)
{
    if (j >= 0) return j;
    else return -j;
}

long labs(long j)
{
    if (j >= 0) return j;
    else return -j;
}

#if ! __x86_64__
/*
 * divide u64/u64 (needed for rdtsc)
 * In 32 bit mode, this can't be done directly and the compiler need this function.
 * Here, we don't use the FPU or SSE, but shift the arguments into 32 bit,
 * divide in 32 bit and restore the result to 64 bits (using number of places shifted before)
 */
uint64_t __udivdi3(uint64_t n, uint64_t d)
{
    uint32_t n32, d32, r32, c=0;
    uint64_t r;
    while (n > 0xFFFFFFFFull) {
        n >>= 1;
        c++;
    }
    n32 = (uint32_t)(n & 0xFFFFFFFFull);
    while (d > 0xFFFFFFFFull && c >= 1) {
        d >>= 1;
        c--;
    }
    d32 = (uint32_t)(d & 0xFFFFFFFFull);
    r32 = n32/d32;
    r = (uint64_t)r32 << c;
    return r;
}

#endif
