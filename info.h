/*
 * =====================================================================================
 *
 *       Filename:  info.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.09.2011 13:51:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef INFO_H
#define INFO_H

#include "stddef.h"
#include "config.h"

typedef enum {vend_unknown, vend_intel, vend_amd} vendor_t;

extern char *vendor_string[];       // in cpu.c

typedef struct {
    uint32_t mb_adr;        /* first position: address of multiboot info structure */

    /* MULTIBOOT */
    uint32_t cmd_maxcpu;
    uint32_t cmd_cpumask;
    uint32_t cmd_noacpi;
    uint32_t cmd_nomps;

    /* CPUID */
    uint32_t cpuid_max;
    uint32_t cpuid_high_max;
    uint32_t cpuid_family;
    uint32_t cpuid_cachelinesize;
    uint32_t cpuid_lapic_id;
    vendor_t cpu_vendor;
    union {
        char c[48];
        uint32_t u32[12];
    } cpuid_processor_name;
    uint16_t cpuid_threads_per_package;
    struct {
        uint8_t level;
        char type;      // Data, Instruction, Unified
        uint8_t shared_by;
        uint8_t line_size;
        uint32_t size;
        // associativity... (n-way)?
    } cpuid_cache[MAX_CACHE];

    /* BDA and EBDA */
    uint32_t ebda_adr;
    uint32_t ebda_size;

    /* CPUs */
    uint32_t cpu_cnt;
    struct {
        uint32_t lapic_id;
    } cpu[MAX_CPU];
    uint32_t lapic_adr;

    /* BUS */
    uint32_t bus_isa_id;        /* what bus-id has the ISA bus (for redirecting the IRQs) */

    /* I/O APICs */
    uint32_t ioapic_cnt;
    struct {
        uint32_t id;
        uint32_t adr;
    } ioapic[MAX_IOAPIC];

    /* PCI-Express Config Space */
    uint32_t pcie_cnt;
    struct {
        ptr_t base_adr;
        uint16_t grp_nbr;
        uint8_t bus_start;
        uint8_t bus_end;
    } pcie_cfg[MAX_PCIE];

    /* TSC */
    //uint64_t tsc_per_sec;
    uint32_t tsc_per_usec;      
    uint32_t usec_per_mtsc;
                                /* ATTN: when using uint64_t (as in tsc_per_sec),
                                 * this structure behaves differently in 32 bit and 64 bit.
                                 * And it is used from boot32.c in 32 bit mode and later in the 64 bit kernel also. 
                                 * this is why __attribute__((packed)) is needed on the struct! */

} __attribute__((packed)) hw_info_t;

extern hw_info_t hw_info; 

#endif // INFO_H

