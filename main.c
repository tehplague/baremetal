#include "system.h"
#include "multiboot_struct.h"
#include "info.h"
#include "smp.h"
#include "keyboard.h"
#include "sync.h"
#include "mm.h"
#include "pit.h"
#include "debug.h"
#include "cpu.h"
#include "pci.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_MAIN > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_MAIN > 1)

hw_info_t hw_info; 


/*
 * TODO's
 *  - is the cache activated?
 *  - dynamic memory management (at least, malloc() should be implemented)
 */
barrier_t mainbarrier = BARRIER_INITIALIZER(MAX_CPU); /* max is later reduced to the actual number of CPUs */
//barrier_t mainbarrier = {.cnt=0,.epoch=0,.max=16}; 
//(barrier_t){ .cnt=0, .epoch=0, .max=16 }; /* max is later reduced to the actual number of CPUs */

extern volatile unsigned cpu_online;    // from apic.c
void main();                            // further down in this file

#if __x86_64__
#define VERSION_BITS "64"
#else
#define VERSION_BITS "32"
#endif
#include "version.h"
static char name_version[] = "BareMetalKernel" VERSION_BITS " " SVN_REV " OPT=" OPT;

/*
 * this is the entry function only for the BSP
 */
void main_bsp(void)
{
    char *vendor[] = {"Intel", "AMD", "unknown"};

    *((uint32_t*)0xB8000) = 0x1F391F39;     /* "99" top left corner to say: "I've arrived in main()." */
    //status_putch(6, '/');

    init_video();
    smp_init();
    IFVV printf("my_cpu_info()->cpu_id: %u\n", my_cpu_info()->cpu_id);


    IFV puts("main(): video initialized\n");
    IFVV printf("found %d %s CPUs and %d I/O APICs\n", 
            (ptr_t)hw_info.cpu_cnt, vendor[hw_info.cpu_vendor], (ptr_t)hw_info.ioapic_cnt);
    //udelay(DELAY);

    mm_init();
    IFVV printf("my_cpu_info()->cpu_id: %u\n", my_cpu_info()->cpu_id);

#if SCROLLBACK_BUF_SIZE
    init_video_scrollback();
#endif

    idt_install();
    IFV puts("idt installed\n");
    //udelay(DELAY);

    isr_install();
    IFV puts("isr installed\n");
    //udelay(DELAY);
    
    pit_init();
    IFV puts("tsc calibrated\n");
    
    apic_init();    /* this is where the APs are waked up */
    IFV puts("apic initialized\n");

    pci_init();
    IFV puts("pci initialized\n");

    keyboard_init(kbm_poll);
    IFV puts("keyboard initialized\n");


    IFV puts("my kernel is running in main_bsp now...\n");

    char *p = name_version;
    unsigned u;
    for (u=40; u<75 && *p != 0; u++) {
        status_putch(u, *p++);
    }

    //unsigned eax, ebx, ecx, edx;
    //cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    //printf("support 1 GB pages: %d\n", edx & (1<<26));
    
    //printf("cpuid_max: 0x%x   cpuid_high_max: 0x%x\n", hw_info.cpuid_max, hw_info.cpuid_high_max);
    //printf("cpuid_family: 0x%x\n", hw_info.cpuid_family);


    //int i; for (i=0; i< 40; i++) printf("Test line %d\n", i);
    //print_multiboot_info();
    //print_smp_iboot32.o nfo();

    IFVV {
        printf("offset of stack[0] : 0x%x ", &(stack[0]));
        ptr_t sp;
#       if __x86_64__
        __asm__ volatile("movq %%rsp, %%rax" : "=a"(sp) );
#       else
        __asm__ volatile("movl %%esp, %%eax" : "=a"(sp) );
#       endif
        printf("my sp: 0x%x ", sp);
        printf("my_cpu_info: 0x%x ", my_cpu_info());
        printf("[%u]\n", my_cpu_info()->cpu_id);
    }
    //printf("new[0]: cpu_info = %x cpu_id = %x\n", my_cpu_info(), my_cpu_info()->cpu_id);
    
    //multiboot_info(); 
    

    cpu_online++;       // the BSP is there, too
    mainbarrier.max = cpu_online;
    /* wait until all others are in the following barrier */
    while (mainbarrier.cnt < (mainbarrier.max-1)) {};
    barrier(&mainbarrier);

    main();
}



/*
 * this is the entry function for the APs.
 */
void main_ap(void)
{
    cpu_online++;
    /*
     * Signal, that the initialization of this CPU is done. 
     * If I'm not here in time, apic_init() will trylock() my wakelock
     * and I will block here.
     */
    mutex_lock(&(my_cpu_info()->wakelock)); 

    apic_init_ap(cpu_online);     // activate localAPIC on Application Processors
    idt_install_ap();

    smp_status(STATUS_RUNNING);

    //udelay(3000000*my_id);
    //printf("new[%d]: cpu_info = %x cpu_id = %x\n", cpu_online, my_cpu_info(), my_cpu_info()->cpu_id);

    barrier(&mainbarrier);
    main();
}


#include "payload.h"
#include "tests.h"

/*
 * this is the main function that is entered by all CPUs after the initialization 
 * (no difference between BSP and AP, anymore)
 */
void main()
{
    IFVV printf("CPU %d/%d entering in main()\n", my_cpu_info()->cpu_id, cpu_online);

    if (my_cpu_info()->cpu_id == 0) {
        printf("*****************************************\n");
        printf("* CPU Vendor: %s \n", vendor_string[hw_info.cpu_vendor]);
        printf("* CPU Name: '%s' \n", hw_info.cpuid_processor_name.c);
        printf("* max CPUID fn: 0x%x, 0x%x\n", hw_info.cpuid_max, hw_info.cpuid_high_max);
        printf("* Nbr of threads/package: %u \n", hw_info.cpuid_threads_per_package);
        printf("* Cache-Line Size: %u \n", (ptr_t)hw_info.cpuid_cachelinesize);
        printf("* L1$: %#uB %c + %#uB %c\n", 
                hw_info.cpuid_cache[0].size, 
                hw_info.cpuid_cache[0].type, 
                hw_info.cpuid_cache[1].size,
                hw_info.cpuid_cache[1].type); 
        printf("* L2$: %#uB\n", hw_info.cpuid_cache[2].size);
        if (hw_info.cpuid_cache[3].size > 0) 
            printf("* L3$: %#uB\n", hw_info.cpuid_cache[3].size);
        printf("*****************************************\n");


    }


    /* call tests */
    //tests_doall();

    /* call a payload */
    payload_benchmark();

    /* all CPUs leaving the payload: go to sleep */
    stop();
}

