#ifndef STDDEF_H
#define STDDEF_H

#if __x86_64__
    typedef unsigned char      u8;
    typedef unsigned short     u16;
    typedef unsigned int       u32;
    typedef unsigned long long u64;
#else
    typedef unsigned char      u8;
    typedef unsigned short     u16;
    typedef unsigned int       u32;
    typedef unsigned long long u64;
#endif

typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef unsigned long size_t;   // size of a pointer; 32/64 bit safe
typedef unsigned long ptr_t;    // size of a pointer; 32/64 bit safe

#define KB  (1024)
#define MB  (1024 * KB)
#define GB  (1024 * MB)

#endif
