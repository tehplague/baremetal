/*
 * =====================================================================================
 *
 *       Filename:  debug.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:01:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "stddef.h"
#include "info.h"
#include "system.h"
#include "lib.h"
#include "cpu.h"
#include "multiboot_struct.h"

void print_multiboot_info(void) 
{
    multiboot_info_t *mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
    char mem_type[][10] = {"mem", "other"};

    printf("Multiboot-Flags: 0x%x\n", mbi->flags);

    if (mbi->flags & (1<<0)) {
        printf("flags[0] - mem_lower: 0x%x=%d  mem_upper: 0x%x=%d\n", mbi->mem_lower, mbi->mem_lower, mbi->mem_upper, mbi->mem_upper);
    }

    if (mbi->flags & (1<<1)) {
        printf("flags[1] - boot_device: %x %x %x %x (Part 3/2/1 / Drive)\n", mbi->boot_device&0xFF, (mbi->boot_device>>8)&0xFF, 
                (mbi->boot_device>>16)&0xFF, (mbi->boot_device>>24)&0xFF);
    }

    if (mbi->flags & (1<<2)) {
        printf("flags[2] - cmdline: '%s'\n", mbi->cmdline);
    }

    if (mbi->flags & (1<<3)) {
        printf("flags[3] - mods_count: %d  mods_addr: 0x%x\n", mbi->mods_count, mbi->mods_addr);
        unsigned i;
        for (i=0; i<mbi->mods_count; i++) {
            printf("  mod[%d]...\n", i);
        }
    }

    if (mbi->flags & (1<<4)) {
        printf("flags[4] - Symbol table for a.out image...\n");
    }

    if (mbi->flags & (1<<5)) {
        printf("flags[5] - Section header table for ELF kernel...\n");
    }

    if (mbi->flags & (1<<6)) {
        printf("flags[6] - mmap_length: %d  mmap_addr: 0x%x\n", mbi->mmap_length, mbi->mmap_addr);
        multiboot_memory_map_t* p = (multiboot_memory_map_t*)(long)mbi->mmap_addr;
        for ( ; p < (multiboot_memory_map_t*)(long)(mbi->mmap_addr+mbi->mmap_length); p = ((void*)p + p->size + 4)) {
            printf("  mmap[0x%x] - addr:0x%x  len:0x%x  type: %d (%s)\n",
                   p, (multiboot_uint32_t)(p->addr), (multiboot_uint32_t)(p->len), p->type, mem_type[p->type==1?0:1]);
        }
    }

    /* more bits available in flags, but their display is not implemented, yet */
    /* see: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format */
}

void print_smp_info(void)
{
    unsigned eax, ebx, ecx, edx;
    volatile unsigned *register_apic_id;
    unsigned i;

    cpuid(0x0B, &eax, &ebx, &ecx, &edx);
    printf("APIC ID from CPUID.0BH:EDX[31:0]: %d\n", edx);

    register_apic_id = (volatile unsigned *) (0xFEE00000 + 0x20);
    i = *register_apic_id;
    i = i >> 24;
    printf("local APIC ID from register: %d\n", i);

    cpuid(0x01, &eax, &ebx, &ecx, &edx);
    printf("support SMP: %d\n", edx & (1<<28));
    printf("addressable logical processors: %d\n", (ebx>>16)&0xFF);

    cpuid2(0x04, 0x00, &eax, &ebx, &ecx, &edx);
    i = (eax>>26)+1;
    printf("addressable processor cores: %d\n", i);
}

static inline void stackdump(int from, int to)
{
    long *sp;
    long *p;
#   ifdef __x86_64__
    __asm__ volatile ("mov %%rsp, %%rax" : "=a"(sp));
#   else
    __asm__ volatile ("mov %%esp, %%eax" : "=a"(sp));
#   endif
    printf("sp: %x\n", sp);
    for (p = sp-to; p <= sp-from; p++) {
        printf("[%x]   %x\n", p, *p);
    }
}


void multiboot_info(void)
{
    multiboot_info_t *p_mbi;
    printf("hw_info.mb_adr = %x\n", hw_info.mb_adr);
    p_mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
    printf("p_mbi->flags = %x\n", p_mbi->flags);
    if (p_mbi->flags & (1<<2)) {
        printf("p_mbi->cmdline = %x ", p_mbi->cmdline);
        char *str = (char*)(ptr_t)p_mbi->cmdline;
        printf("'%s'\n", str);
    }
    printf("hw_info->cmd_maxcpu = %u\n", hw_info.cmd_maxcpu);
    printf("hw_info->cmd_cpumask = %u\n", hw_info.cmd_cpumask);
}



/* deactivate warning on divide-by-zero, b/c this is intentional in this function */
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
void test_div_zero()
{
    printf("DIV ZERO in 1 sec ");
    udelay(100000);
    printf("9");
    udelay(100000);
    printf("8");
    udelay(100000);
    printf("7");
    udelay(100000);
    printf("6");
    udelay(100000);
    printf("5");
    udelay(100000);
    printf("4");
    udelay(100000);
    printf("3");
    udelay(100000);
    printf("2");
    udelay(100000);
    printf("1");
    udelay(100000);
    printf("0");
    udelay(100000);
    printf("1/0 = %d", 1/0);    /* divide by zero to test DIVZERO exception */
    printf("\nafter DIV ZERO\n");
    udelay(500000);
}

