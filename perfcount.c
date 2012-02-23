/*
 * =====================================================================================
 *
 *       Filename:  perfcount.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.02.2012 10:02:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Christian Spoo (cs) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "perfcount.h"
#include "system.h"
#include "cpu.h"
#include "info.h"

static uint32_t __perfcount_perfevtsel_nehalem[4] = {0x186u, 0x187u, 0x188u, 0x189u};
static uint32_t __perfcount_pmc_nehalem[4]        = {0x0C1u, 0x0C2u, 0x0C3u, 0x0C4u};
static uint32_t __perfcount_globalctrl_nehalem    = 0x38Fu;

static void (*__perfcount_start)(uint8_t) = NULL;
static void (*__perfcount_stop)(uint8_t) = NULL;
static void (*__perfcount_reset)(uint8_t) = NULL;
static uint64_t (*__perfcount_read)(uint8_t) = NULL;

static void __perfcount_init_nehalem(uint8_t counter, uint64_t config);
static void __perfcount_start_nehalem(uint8_t counter);
static void __perfcount_stop_nehalem(uint8_t counter);
static void __perfcount_reset_nehalem(uint8_t counter);
static uint64_t __perfcount_read_nehalem(uint8_t counter);

static void __perfcount_init_nehalem(uint8_t counter, uint64_t config) {
  uint64_t event_sel;
  uint64_t global_ctrl;

  if (counter >= 3) {
    printf("Nehalem only has 4 performance counters\n");
    return;
  }

  // Map common configuration values to Nehalem ones (very simple mapping as Nehalem is the architecture that is mostly used)
  if (config >= 0xFF000000ull)
    config &= 0xFFFFull;
  event_sel = config | (1 << 17 /* IA32_PERFEVTSEL_OS */) | (1 << 16 /* IA32_PERFEVTSEL_USR */); 

  // Initialize PMC
  global_ctrl = rdmsr(__perfcount_globalctrl_nehalem);
  global_ctrl |= (1 << counter);
  wrmsr(__perfcount_globalctrl_nehalem, global_ctrl);

  // Write but leave the counter in disabled state
  wrmsr(__perfcount_perfevtsel_nehalem[counter], event_sel);
  
  // Reset PMC
  __perfcount_reset_nehalem(counter);
}

static void __perfcount_start_nehalem(uint8_t counter) {
  if (counter >= 3) {
    printf("Nehalem only has 4 performance counters\n");
    return;
  }

  uint64_t event_sel = rdmsr(__perfcount_perfevtsel_nehalem[counter]);
  event_sel |= (1 << 22 /* IA32_PERFEVTSEL_EN */);
  wrmsr(__perfcount_perfevtsel_nehalem[counter], event_sel);
}

static void __perfcount_stop_nehalem(uint8_t counter) {
  if (counter >= 3) {
    printf("Nehalem only has 4 performance counters\n");
    return;
  }

  uint64_t event_sel = rdmsr(__perfcount_perfevtsel_nehalem[counter]);
  event_sel &= ~(1 << 22 /* IA32_PERFEVTSEL_EN */);
  wrmsr(__perfcount_perfevtsel_nehalem[counter], event_sel);
}

static void __perfcount_reset_nehalem(uint8_t counter) {
  if (counter >= 3) {
    printf("Nehalem only has 4 performance counters\n");
    return;
  }

  wrmsr(__perfcount_pmc_nehalem[counter], 0);
}

static uint64_t __perfcount_read_nehalem(uint8_t counter) {
  if (counter >= 3) {
    printf("Nehalem only has 4 performance counters\n");
    return 0;
  }

  return rdmsr(__perfcount_pmc_nehalem[counter]);
}

void perfcount_init(unsigned int counter, uint64_t config) {
  if (hw_info.cpu_vendor == vend_intel) {
    __perfcount_start = __perfcount_start_nehalem;
    __perfcount_stop = __perfcount_stop_nehalem;
    __perfcount_reset = __perfcount_reset_nehalem;
    __perfcount_read = __perfcount_read_nehalem;

    __perfcount_init_nehalem(counter, config);
  } 
  else {
    printf("perfcount: warning: PMC either unsupported, but at least not implemented\n");
    printf("perfcount: do not rely on returned results\n");
  }
}

uint64_t perfcount_raw(uint8_t event, uint8_t umask) {
  return (uint64_t)(umask | (event << 8));
}

void perfcount_start(unsigned int counter) {
  if (!__perfcount_start) {
    printf("perfcount: warning: PMC is not initialized or unsupported\n");
    return;
  }

  __perfcount_start(counter);
}

void perfcount_stop(unsigned int counter) {
  if (!__perfcount_stop) {
    printf("perfcount: warning: PMC is not initialized or unsupported\n");
    return;
  }

  __perfcount_stop(counter);
}

void perfcount_reset(unsigned int counter) {
  if (!__perfcount_reset) {
    printf("perfcount: warning: PMC is not initialized or unsupported\n");
    return;
  }

  __perfcount_reset(counter);
}

uint64_t perfcount_read(unsigned int counter) {
  if (!__perfcount_read) {
    return 0; 
  }

  return __perfcount_read(counter);
}



