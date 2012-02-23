/*
 * =====================================================================================
 *
 *       Filename:  smm.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 16:05:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "stddef.h"
#include "system.h"
#include "config.h"
#include "cpu.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_SMM > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_SMM > 1)

#define IN(var, port) __asm__ volatile ("in %%dx, %%eax" : "=a" (var) : "d" (port))
#define OUT(port, var) __asm__ volatile ("out %%eax, %%dx" :  : "a" (var), "d" (port))
#define LPC_BUS    0x00
#define LPC_DEVICE 0x1F
#define LPC_FUNC   0x00
#define LPC_OFFSET 0x40
#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC
static uint32_t bak_smi_en;
static uint32_t pmbase = 0;             

void smm_deactivate(void)
{
    // TODO: how to read pmbase correctly from pci_config? Are there MetalSVM-functions to do this?
    uint32_t adr;
    uint32_t smi_en;
    unsigned if_backup;

    if_backup = cli();

    /*
    union {
        char string[48];
        uint32_t reg[12];
    } brand;

    __asm__ volatile ("movl $0x80000002, %%eax \n\t cpuid" : "=a"(brand.reg[0]), "=b"(brand.reg[1]), "=c"(brand.reg[2]), "=d"(brand.reg[3]));
    __asm__ volatile ("movl $0x80000003, %%eax \n\t cpuid" : "=a"(brand.reg[4]), "=b"(brand.reg[5]), "=c"(brand.reg[6]), "=d"(brand.reg[7]));
    __asm__ volatile ("movl $0x80000004, %%eax \n\t cpuid" : "=a"(brand.reg[8]), "=b"(brand.reg[9]), "=c"(brand.reg[10]), "=d"(brand.reg[11]));
    printf("brand string: '%s'\n", brand.string);
    */

    adr = (1<<31) | (LPC_BUS << 16)  |  (LPC_DEVICE << 11)  |  (LPC_FUNC <<  8)  |  LPC_OFFSET;
    OUT(CONFIG_ADDRESS, adr);
    IN(pmbase, CONFIG_DATA);
    pmbase &= 0xFF80;
    IFVV printf("pmbase : 0x%x \n", pmbase);



    adr = pmbase+0x30;                          /* SMI_EN I/O register is at pmbase+0x30 */

    /* read SMI_EN and display */
    IN(smi_en, adr);
    bak_smi_en = smi_en;
    IFVV printf("SMI_EN: 0x%x \n", smi_en);

    /* set to 0 (only writable bits will be changed...) */
    smi_en = 0; //&= ~1;
    OUT(adr, smi_en);

    /* read again to see what has changed */
    IN(smi_en, adr);
    IFVV printf("SMI_EN: 0x%x (after setting SMI_EN to 0; bit 0 is GBL_SMI_EN)\n", smi_en);

    /* if GBL_SMI_EN (bit #0) is 0, deactivation was successful */
    if ((smi_en & 0x01) == 0) {
        IFV printf("SMI globally disabled\n");
    } else {
        printf("Warning: SMI was not disabled!\n");
    }
    if (if_backup) sti();
}

void smm_restore(void)
{
    if (pmbase != 0)
        OUT(pmbase+0x30, bak_smi_en);
}


