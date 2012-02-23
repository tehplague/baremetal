/*
 * =====================================================================================
 *
 *       Filename:  tests.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.11.2011 09:19:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "system.h"
#include "smp.h"
#include "sync.h"
#include "mm.h"
#include "cpu.h"
#include "keyboard.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_TESTS > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_TESTS > 1)

extern volatile unsigned cpu_online;

static barrier_t barr_all = BARRIER_INITIALIZER(MAX_CPU+1);

void tests_barrier(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    unsigned u;
    static barrier_t barr2 = BARRIER_INITIALIZER(2);

    if (cpu_online >= 2 && myid < 2) {
        barrier(&barr2);
        for (u=0; u<20; u++) {

            if ((u % (myid+1)) == 0) udelay(1000*(u+1));

            barrier(&barr2);

        }
        barrier(&barr2);
    }

    barrier(&barr_all);
    for (u=0; u<20; u++) {

        if ((u % (myid+1)) == 0) udelay(1000*(u+1));

        barrier(&barr_all);
    }
    IFV printf("[%u] leaving test_barrier()\n", myid);
    barrier(&barr_all);
}

void tests_flag(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    static flag_t flag = FLAG_INITIALIZER;
    
    barrier(&barr_all);

    if (myid == 0) {
        udelay(1000000);
        printf("[0] signal flag\n");
        flag_signal(&flag);

        udelay(1000000);
        printf("[0] signal second flag\n");
        flag_signal(&flag);

        udelay(2000000);
        printf("[0] signal third flag\n");
        flag_signal(&flag);

    } else if (myid == 1) {
        flag_wait(&flag);
        printf("[1] detected flag\n");
        flag_wait(&flag);
        printf("[1] detected second flag\n");

        while (!flag_trywait(&flag)) {
            udelay(100000);
            printf("z");
        }
        printf("[1] detected third flag\n");

    }

}

void tests_mm(void)
{
    static barrier_t barr = BARRIER_INITIALIZER(2);
    unsigned myid = my_cpu_info()->cpu_id;
    static volatile uint32_t * volatile p_shared = NULL;
    static volatile uint32_t * volatile p_shared2 = NULL;
    static volatile uint32_t * volatile p_shared3 = NULL;
    static volatile uint32_t * volatile p_shared4 = NULL;

    if (cpu_online >= 2) {
        if (myid == 0) {
            /* call Task for CPU 0 */
            p_shared = heap_alloc(1, 0);   // one page = 4kB
            p_shared2 = heap_alloc(4, 0);   // one page = 16kB
            printf("[0] p_shared = 0x%x\n", p_shared);
            printf("[0] p_shared2 = 0x%x\n", p_shared2);
            udelay(1*1000*1000);
            barrier(&barr);
            p_shared3 = heap_alloc(2, 0);   // two pages = 8kB
            barrier(&barr);
            printf("p_shared[1023] = 0x%x (should be 0x01010101)\n", p_shared[1023]);
            printf("p_shared2[2048] = 0x%x (should be 0x22222222)\n", p_shared2[2048]);

            barrier(&barr);
            p_shared[0] = 0xdeadbeef;
            p_shared2[0] = 0xdeadbeef;
            p_shared3[0] = 0xdeadbeef;
            barrier(&barr);
            p_shared4[1] = 0xdeadc0de;

            printf("CPU 0: udelay 5 Sek.\n");
            udelay(5*1000*1000);
            printf("CPU 0: exit now\n");
        } else if (myid == 1) {
            /* call Task for CPU 1 */
            barrier(&barr);
            p_shared4 = heap_alloc(2, 0);   // two pages = 8kB
            udelay(1*1000*1000);
            printf("[1] p_shared = 0x%x\n", p_shared);
            printf("[1] p_shared2 = 0x%x\n", p_shared2);
            printf("[1] p_shared3 = 0x%x\n", p_shared3);
            printf("[1] p_shared4 = 0x%x\n", p_shared4);
            udelay(1*1000*1000);
            memset((void*)p_shared, 1, 4096);
            memset((void*)p_shared2, 0x22, 4*4096);
            barrier(&barr);
            
            barrier(&barr);
            p_shared4[0] = 0xdeadbeef;
            barrier(&barr);
            p_shared[1] = 0xdeadc0de;
            p_shared2[1] = 0xdeadc0de;
            p_shared3[1] = 0xdeadc0de;

            printf("CPU 1: udelay 10 Sek.\n");
            udelay(10*1000*1000);
            printf("CPU 1: exit now\n");
        } 
    } else {
        printf("only one CPU active, this task needs at least two.\n");
    }

}
void tests_mm_reconf()
{
    unsigned myid = my_cpu_info()->cpu_id;
    void *p_buffer = 0;
    size_t size = 16*KB;        // 4 pages, fits into L1$

    void membench() 
    {
        unsigned i, j;
        uint64_t t1, t2;
        volatile uint32_t *p = (volatile uint32_t*)p_buffer;
        t1 = rdtsc();
        for (i=0; i<4096; i++) {
            for (j=0; j<size; j += 4) {
                p[10]++;
            }
        }
        t2 = rdtsc();
        printf("membench: %u tics/access\n", (t2-t1)/4096/(size/4));
    }

    if (myid == 0) {
        udelay(100000);
        p_buffer = heap_alloc(size/PAGE_SIZE, MM_CACHE_DISABLE);
        membench();
        membench();
        heap_reconfig(p_buffer, size, MM_WRITE_THROUGH);
        membench();
        membench();
        heap_reconfig(p_buffer, size, 0);
        membench();
        membench();
        heap_reconfig(p_buffer, size, MM_CACHE_DISABLE);
        membench();
        membench();

    }


}

void tests_ipi(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    unsigned if_backup;

    if_backup = sti();
    if (cpu_online > 1) {

        if (myid == 0) {
            unsigned u;

            //__asm__ volatile ("int $31");
            
            IFVV printf("issue INT 128...\n");
            __asm__ volatile ("int $128");
            udelay(2000000);
            
            IFVV printf("send IPI vector 128 to self\n");
            send_ipi(0, 0x80);

            for (u = 1; u < cpu_online; u++) {
                udelay(1000000);
                IFVV printf("send IPI vector %u to CPU %u \n", 0x80, u);
                send_ipi(u, 0x80);
            }

            for (u = 1; u < cpu_online; u++) {
                udelay(1000000);
                IFVV printf("send IPI vector %u to CPU %u \n", 0x80, u);
                smp_wakeup(u);
            }
        } else {

            if (myid == 1) {
                udelay(1000000);
                IFVV printf("issue INT 128 on CPU 1\n");
                __asm__ volatile ("int $128");
            }

            smp_status('H');
            __asm__ volatile ("hlt");   // should be waken up by IPI, but apparantly, IS NOT.
            smp_status(STATUS_RUNNING);
            
            smp_halt();
        }

    } else {
        printf("tests_ipi: can only be executed with more than one CPU\n");
    }
    if (!if_backup) cli();  // restore previous state of interrupt flag
    barrier(&barr_all);
}

void tests_printf(void)
{
    printf("\n");
    printf("u:'%u', x:'%x', k:'%#u', u:'%u', k:'%#u', u:'%u'\n", 123, 123, 123, 123, 123, 123);
    printf("u:'%u', x:'%x', k:'%#u', #x:'%#x', k:'%#u', u:'%u'\n", 123, 123, 123, 123, 123, 123);
    printf("u:'%u', x:'%x', k:'%#10u', #8x:'%#8x', k:'%#u', u:'%u'\n", 123, 123, 123, 123, 123, 123);
    for (unsigned u=512; u<128*MB; u*=8) printf("%10u: %#uB\n", u, u);
}
void tests_keyboard(void)
{
    unsigned u;
    unsigned scancode, keycode;
    printf("polling ~30 secs for keyboard scan codes (press some keys...)\n");
    for (u=0; u<64; u++) {
        scancode = keyboard_get_scancode();
        printf("%x ", scancode);
        udelay(500*1000);
    }
    printf("\n");

    printf("polling ~30 secs for keyboard input (press some keys...)\n");
    for (u=0; u<64; u++) {
        scancode = keyboard_get_scancode();
        keycode = scancode_to_keycode(scancode);
        if (keycode) putch(keycode); 
        printf(".");
        udelay(500*1000);
    }
    printf("\n");

    printf("press any key...");
    keycode = wait_for_key();
    putch((uint8_t)keycode);
    printf("\n");

}

void tests_doall(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    if (myid == 0) {
        barr_all.max = cpu_online;
        /* wait until all others are in the following barrier */
        while (barr_all.cnt < (barr_all.max-1)) { };
    }
    barrier(&barr_all);

    IFV printf("[%u] calling test_barrier()\n", myid);
    //tests_barrier();
    //tests_flag();

    //tests_mm();
    tests_mm_reconf();

    //tests_ipi();

    //if (myid == 0) {
    // tests_printf();
    // tests_keyboard();
    //}
    
    /*unsigned u;
    for (u=0; u<30; u++) {
        printf("[%u] line %u\n", myid, u);
    }*/

    printf("[%u] exit tests_doall()\n", myid);
}

