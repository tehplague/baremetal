/*
 * =====================================================================================
 *
 *       Filename:  payload.c
 *
 *    Description:  This file contains some payloads to execute after the kernel has booted
 *
 *        Version:  1.0
 *        Created:  20.10.2011 09:21:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "sync.h"
#include "system.h"
#include "smp.h"
#include "mm.h"
#include "benchmark.h"
#include "smm.h"

extern volatile unsigned cpu_online;

/*
 * The payload is called by all CPUs after complete initialization.
 * A Barrier is executed immediately before, so they should come in shortly.
 */

static mutex_t mut = MUTEX_INITIALIZER;
static barrier_t barr = BARRIER_INITIALIZER(MAX_CPU+1); // this barrier will be set to cpu_online
//static barrier_t barr2 = BARRIER_INITIALIZER(2);        // barrier for two
//static flag_t flag = FLAG_INITIALIZER;

void payload_benchmark()
{
    unsigned myid = my_cpu_info()->cpu_id;

    /*
     * count and collect all processors (collective barrier)
     */
    mutex_lock(&mut);
    if (barr.max == MAX_CPU+1) {
        /* first one sets barr.max to the actual count of CPUs */
        barr.max = cpu_online;
        smm_deactivate();       // ...and (try to) deactivate SMM
    }
    mutex_unlock(&mut);

    size_t buffer_size = 16 * MB;
    static void *p_buffer = NULL;

    /*
     * Memory allocation
     */
    if (myid == 0) {
        p_buffer = heap_alloc(buffer_size / PAGE_SIZE, BENCH_WORK_FLAGS);       // one page = 4kB
        /* no need for pre-faulting, because pages are present after head_alloc()
         * (we don't have demand paging)
         * but initialize them */
        memset(p_buffer, 0, buffer_size);
    }

    size_t contender_size = 16 * MB;
    static void *p_contender = NULL;

    if (myid == 0) {
        p_contender = heap_alloc(contender_size / PAGE_SIZE, BENCH_LOAD_FLAGS);       // one page = 4kB
        //virt_to_phys(p_contender);
        //p_contender[0] = 42;
        //printf("[1] p_contender = 0x%x .. 0x%x\n", (ptr_t)p_contender, (ptr_t)p_contender+16*1024*1024);
        memset(p_contender, 0, contender_size);
    }


    /*
     *   Benchmarks
     */

    //bench_hourglass(&barr);
    //bench_hourglass_worker(&barr, p_contender);
    //bench_hourglass_hyperthread(&barr);

    barrier(&barr);

    //bench_worker(&barr, p_buffer, p_contender);
    bench_worker_cut(&barr, p_buffer, p_contender, 16*KB);
    
    if (myid == 0) {
        heap_reconfig(p_buffer, buffer_size, 0);
        heap_reconfig(p_contender, contender_size, MM_CACHE_DISABLE);
        barrier(&barr);
        printf("========  Benchmark: WB / Load: CD ===================================\n");
    } else {
        barrier(&barr);
        tlb_shootdown(p_buffer, buffer_size);
        tlb_shootdown(p_contender, contender_size);
    }
    barrier(&barr);

    bench_worker_cut(&barr, p_buffer, p_contender, 16*KB);
    //bench_worker_cut(&barr, p_buffer, p_contender, 128*KB);

    if (myid == 0) {
        heap_reconfig(p_buffer, buffer_size, 0);
        heap_reconfig(p_contender, contender_size, MM_WRITE_THROUGH);
        barrier(&barr);
        printf("========  Benchmark: WB / Load: WT ===================================\n");
    } else {
        barrier(&barr);
        tlb_shootdown(p_buffer, buffer_size);
        tlb_shootdown(p_contender, contender_size);
    }
    barrier(&barr);

    bench_worker_cut(&barr, p_buffer, p_contender, 16*KB);
    //bench_worker_cut(&barr, p_buffer, p_contender, 128*KB);

    barrier(&barr);

    //bench_mem(&barr, p_buffer, p_contender);

}

void payload_demo()
{
    unsigned myid = my_cpu_info()->cpu_id;
    mutex_lock(&mut);
    if (barr.max == MAX_CPU+1) {
        /* first one sets barr.max to the actual count of CPUs */
        barr.max = cpu_online;
    }
    mutex_unlock(&mut);

    /* needs at least two CPUs */
    if (cpu_online >= 2) {
        if (myid == 0) {
            /* call Task for CPU 0 */
            barrier(&barr);

            printf("CPU 0: udelay 5 Sek.\n");
            udelay(5*1000*1000);
            printf("CPU 0: exit now\n");
        } else if (myid == 1) {
            /* call Task for CPU 1 */
            barrier(&barr);

            printf("CPU 1: udelay 10 Sek.\n");
            udelay(10*1000*1000);
            printf("CPU 1: exit now\n");
        } 
    } else {
        printf("only one CPU active, this task needs at least two.\n");
    }
}

