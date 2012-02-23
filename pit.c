/*
 * PIT
 */

#include "system.h"
#include "cpu.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_PIT > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_PIT > 1)

enum PIT {
    PIT_FREQ = 1193182, // Hz
    PIT_CHANNEL0 = 0x40,    // timer
    PIT_CHANNEL2 = 0x42,    // PC speaker
    PIT_MCR = 0x43  // mode and command register (write only, ready ignored)
};

#define PIT_CMD_CHANNEL0        (0<<6)
#define PIT_CMD_CHANNEL1        (1<<6)
#define PIT_CMD_CHANNEL2        (2<<6)
#define PIT_CMD_RW_LSB          (1<<4)
#define PIT_CMD_RW_MSB          (2<<4)
#define PIT_CMD_RW_LSBMSB       (3<<4)
#define PIT_CMD_MODE_INTERRUPT  (0<<1)
#define PIT_CMD_MODE_ONE_SHOT   (1<<1)
#define PIT_CMD_MODE_RATE_GEN   (2<<1)
#define PIT_CMD_MODE_SQUARE     (3<<1)
#define PIT_CMD_MODE_SW_STROBE  (4<<1)
#define PIT_CMD_MODE_HW_STROBE  (5<<1)
#define PIT_CMD_BCD             (1<<0)

static uint64_t PIT_get_tsc_per_xxx(void) {
    uint16_t counter = 0x00;    // 0 = 65.536 ?
    uint64_t tsc_start, tsc_end;
    unsigned long count;            // x86_64 : 64 bit, x86_32 : 32 bit
    unsigned long loopcnt = 0;

    // setup PIT
    /* 0x34 = 0b0011'0100
     * Bit/s        Usage
     *  7            Output pin state                                           0
     *  6            Null count flags                                           0
     *  4 and 5      Access mode :                                              1 1
     *                  0 0 = Latch count value command
     *                  0 1 = Access mode: lobyte only
     *                  1 0 = Access mode: hibyte only
     *                  1 1 = Access mode: lobyte/hibyte                        (*)
     *  1 to 3       Operating mode :                                           0 1 0
     *                  0 0 0 = Mode 0 (interrupt on terminal count)
     *                  0 0 1 = Mode 1 (hardware re-triggerable one-shot)
     *                  0 1 0 = Mode 2 (rate generator)                         (*)
     *                  0 1 1 = Mode 3 (square wave generator)
     *                  1 0 0 = Mode 4 (software triggered strobe)
     *                  1 0 1 = Mode 5 (hardware triggered strobe)
     *                  1 1 0 = Mode 2 (rate generator, same as 010b)
     *                  1 1 1 = Mode 3 (square wave generator, same as 011b)
     *  0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD     0
     */
    //outportb(PIT_MCR, 0x34);    // channel 0, lo/hi byte order, mode 2 (rate generator), binary counter format (not BCD)
    outportb(PIT_MCR, PIT_CMD_CHANNEL0 | PIT_CMD_RW_LSBMSB | PIT_CMD_MODE_RATE_GEN );    
    //udelay(1); // wait until port is ready (needed on real hardware)
    outportb(PIT_CHANNEL0, (uint8_t) (counter & 0xFF)); // low byte
    //udelay(1); // wait until port is ready (needed on real hardware)
    outportb(PIT_CHANNEL0, (uint8_t) ((counter >> 8) & 0xFF)); // high byte
       
    tsc_start = rdtsc();    // overhead not measured because very small compared to TSC difference

    while (1) {
        loopcnt++;
#       if __x86_64__
            __asm__ volatile (
                "xor %%rax, %%rax\n\t"  // set RAX to 0
                "mov 0x00, %%al\n\t" // channel 0, latch command  (1)
                "out %%al, $0x43\n\t"    // prevent the current count from being updated
                "in $0x40, %%al\n\t" // low byte of current count in AL
                "mov %%al, %%ah\n\t"
                "in $0x40, %%al\n\t"    // high byte of current count in AL
                "rol $8, %%ax\n\t"  // correct order
                : "=a" (count));
#       else    // __x86_32__
            __asm__ volatile (
                "xor %%eax, %%eax\n\t"  // set RAX to 0
                "mov 0x00, %%al\n\t" // channel 0, latch command (1)
                "out %%al, $0x43\n\t"    // prevent the current count from being updated
                "in $0x40, %%al\n\t" // low byte of current count in AL
                "mov %%al, %%ah\n\t"
                "in $0x40, %%al\n\t"    // high byte of current count in AL
                "rol $8, %%ax\n\t"  // correct order
                : "=a" (count));
#       endif   // __x86_??__

            /*
             * (1)
             * this line should read "mov $0, %al" (or, in Intel Syntax: "mov al, 0")
             * but somehow, this does not work on real hardware.
             * The line, as it is, loads effectively the byte from virtual address 0 into AL.
             * (Same as "mov (0x00000000), %al" in AT&T Syntax.)
             * On QEmu, it emits 0x53, on xaxis this is 0xFD.
             * These values both lead to correct results for /count/.
             * If this line is changed to really loading 0 into AL (which is already 0 because of "XOR rax, rax")
             * or if this line is removed,
             * the /count/ always returns 0 on xaxis.
             * Don't know why, but it works as it is.
             * PIT on osdev.org: http://wiki.osdev.org/PIT
             */

        if (count <= (65536ul - 19549ul) ) {    // 1193182 * (1<<14) / 1000000 = 19549.09 (ticks per 2^14 usec = 16000 usec = 16 msec)
            tsc_end = rdtsc();
            IFVV printf("PIT ticks: count=%u 64k-count=%u [%u] ", count, (65536ul-count), loopcnt);
            break;
        }
    }

    // TODO : deactivate PIT!


    return (tsc_end - tsc_start) ;
}

/*
 * average TSC per second
 */
static void PIT_measure_tsc_per_sec(void) {
    const uint64_t pot = 4;             // cycles: power-of-two
    unsigned cycles = (1 << pot);       // 2^4 = 16
    unsigned c;
    uint64_t res, sum = 0;

    cli();  // no interrupts!

    if (cpuid_edx(0x80000007) & (1 << 8)) {
        IFVV printf("TSC is invariant!\n");
    } else {
        IFVV printf("TSC is variant!\n");
    }

    IFVV printf("TSC per usec (preset): %u\n", (unsigned long)(hw_info.tsc_per_usec));

    // TODO: use linear regression to increase accuracy
    for (c = 0; c < cycles; c++) {
        res = PIT_get_tsc_per_xxx();
        IFVV printf("TSCs per xsec: %u\n", res );
        sum += res;
    }

    IFV printf("sum: %u\n", sum);
    hw_info.tsc_per_usec = (sum >> (pot+14));
    IFV printf("TSC per usec calibrated: %u\n", (unsigned long)(hw_info.tsc_per_usec));
    if (hw_info.tsc_per_usec < 1000) {
        hw_info.tsc_per_usec = 2666;
        printf("fall-back: TSC per usec: %u\n", (unsigned long)(hw_info.tsc_per_usec));
    }
    //hw_info.tsc_per_sec = hw_info.tsc_per_usec*1000000;
    hw_info.usec_per_mtsc = 0;
    sum = hw_info.tsc_per_usec;
    while (sum < 1024*1024) {
        sum += hw_info.tsc_per_usec;
        hw_info.usec_per_mtsc++;
    }
    IFV printf("usec per MiSec calibrated: %u\n", (unsigned long)(hw_info.usec_per_mtsc));
    
}

void pit_init(void)
{
    PIT_measure_tsc_per_sec();
}
