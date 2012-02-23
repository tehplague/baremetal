/*
 * This is 32 bit C code for help during the boot sequence.
 * It is used for the 64 bit kernel but should be suited for the 32 bit kernel, too.
 * This is called from startXX.asm in REAL MODE (physical addresses)
 */

#include "multiboot_struct.h"
#include "bda_struct.h"
#include "acpi_struct.h"
#include "mps_struct.h"
#include "info.h"
#include "lib.h"

#include "config.h"
#define IFV   if (VERBOSE > 0 || VERBOSE_BOOT > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_BOOT > 1)

#define MP_FLT_SIGNATURE 0x5f504d5f



/* scrn.c */
void cls();
void putch(char c);
void status_putch(int x, int c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
void itoa (char *buf, int base, long d);
void printf (const char *format, ...);

static inline void halt()
{
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/*
 * read CPU features (mostly from CPUID)
 * see: http://osdev.berlios.de/cpuid.html
 */

static uint16_t * pStatus = (void*)0xB8004;

static void cpu_features()
{
    uint32_t eax, ebx, ecx, edx;
    union {
        char str[13];
        uint32_t u32[3];
    } vendor;
    inline void cpuid(uint32_t func)
    {
        __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func));
    }
    inline void cpuid_ext(uint32_t func, uint32_t ext)
    {
        __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func), "c"(ext));
    }

    *pStatus = 0x0F00 + 'a';

    /* it was checked before, that the CPUID instruction is available */
    *pStatus = 0x0F00 + 'A';
    cpuid(0);                   // same for Intel and AMD
    hw_info.cpuid_max = eax;    // EAX          - maximum valid number vor CPUID(x)
    vendor.u32[0] = ebx;        // EBX:EDX:ECX  - Vendor String
    vendor.u32[1] = edx;
    vendor.u32[2] = ecx;
    vendor.str[12] = 0;
    IFVV printf("vendor: %s\n", vendor.str);

    *pStatus = 0x0F00 + 'H';
    if (strcmp(vendor.str, "GenuineIntel") == 0) 
        hw_info.cpu_vendor=vend_intel;
    else if (strcmp(vendor.str, "AuthenticAMD") == 0) 
        hw_info.cpu_vendor=vend_amd;
    else {
        hw_info.cpu_vendor=vend_unknown;
        //printf("Vendor currently not supported: '%s'.\n", vendor.str);
        //halt();
    }
    *pStatus = 0x0F00 + 'b';

    if (hw_info.cpuid_max >= 1) {
        cpuid(1);
        if (hw_info.cpu_vendor == vend_intel) {
            hw_info.cpuid_family = BITS_FROM_CNT(eax, 8, 4) + BITS_FROM_CNT(eax, 20, 8);
            /*
             * EDX, ECX: Feature Flags
             */
            if ( edx & (1<<19) ) {
                hw_info.cpuid_cachelinesize = BITS_FROM_CNT(ebx, 8, 8) * 8;
            }
            hw_info.cpuid_lapic_id = BITS_FROM_CNT(ebx, 24, 8);    /* only on P4 and later */
        } else if (hw_info.cpu_vendor == vend_amd) {
            hw_info.cpuid_family = BITS_FROM_CNT(eax, 8, 4);
            if (hw_info.cpuid_family == 0x0F) {
                hw_info.cpuid_family += BITS_FROM_CNT(eax, 20, 8);
            }
            // EBX[15:8] : Cache Line Size in Quadwords (8 Bytes)
            hw_info.cpuid_cachelinesize = BITS_FROM_CNT(ebx, 8, 8) * 8;

        }
    }
    printf("cache line size: %u, local APIC id: %u\n", hw_info.cpuid_cachelinesize, hw_info.cpuid_lapic_id);

    *pStatus = 0x0F00 + 'c';

    cpuid(0x80000000);
    hw_info.cpuid_high_max = eax;

    *pStatus = 0x0F00 + 'd';

    if (hw_info.cpuid_high_max >= 0x80000004) {
        cpuid(0x80000002);
        hw_info.cpuid_processor_name.u32[0] = eax;
        hw_info.cpuid_processor_name.u32[1] = ebx;
        hw_info.cpuid_processor_name.u32[2] = ecx;
        hw_info.cpuid_processor_name.u32[3] = edx;
        cpuid(0x80000003);
        hw_info.cpuid_processor_name.u32[4] = eax;
        hw_info.cpuid_processor_name.u32[5] = ebx;
        hw_info.cpuid_processor_name.u32[6] = ecx;
        hw_info.cpuid_processor_name.u32[7] = edx;
        cpuid(0x80000004);
        hw_info.cpuid_processor_name.u32[8] = eax;
        hw_info.cpuid_processor_name.u32[9] = ebx;
        hw_info.cpuid_processor_name.u32[10] = ecx;
        hw_info.cpuid_processor_name.u32[11] = edx;
    }

    *pStatus = 0x0F00 + 'e';

    /*
     * get number of cores/logical threads per package
     *  - Intel: Function 1
     *  - AMD: Function 0x8000_0008
     */
    if (hw_info.cpu_vendor == vend_intel) {
        cpuid(1);
        if (IS_BIT_CLEAR(edx, 28)) {    // HTT
            hw_info.cpuid_threads_per_package = 1;
            printf("Intel w/o HTT: nbr of threads/package: %u\n", hw_info.cpuid_threads_per_package);
        } else {
            hw_info.cpuid_threads_per_package = 1;
            if (hw_info.cpuid_max >= 4) {
                cpuid_ext(4, 0);
                hw_info.cpuid_threads_per_package = BITS_FROM_CNT(eax, 26, 6) +1;
                printf("Intel w/ HTT: nbr of threads/package: %u\n", hw_info.cpuid_threads_per_package);
            }
        }

    } else if (hw_info.cpu_vendor == vend_amd) {
        cpuid(1);
        if (IS_BIT_CLEAR(edx, 28)) {    // HTT
            hw_info.cpuid_threads_per_package = 1;
            printf("AMD w/o HTT: nbr of threads/package: %u\n", hw_info.cpuid_threads_per_package);
        } else {
            hw_info.cpuid_threads_per_package = 1;
            if (hw_info.cpuid_high_max >= 0x80000008) {
                cpuid(0x80000001);
                if (IS_BIT_SET(ecx, 1)) {
                    /* has CmpLegacy */
                    cpuid(0x80000008);
                    hw_info.cpuid_threads_per_package = BITS_FROM_CNT(ecx, 0, 8) +1;
                    printf("AMD w/ CmpLegacy: nbr of threads/package: %u\n", hw_info.cpuid_threads_per_package);
                } else {
                    printf("AMD w/o CmpLegacy: nbr of threads/package: %u\n", hw_info.cpuid_threads_per_package);
                }
            }
        }
    }

    *pStatus = 0x0F00 + 'f';

    /*
     * TODO : read info about cache
     *  - Intel: Function 4 (advanced), alternatively Function 2
     *  - AMD: Functions 0x8000_0005 and 0x8000_0006
     */
    if (hw_info.cpu_vendor == vend_intel) {
        if (hw_info.cpuid_max >= 0x04) {
            /* use Function 4 */
            unsigned u = 0;
            unsigned way, partition, set;
            while (1) {
                cpuid_ext(0x04, u);
                if (BITS_FROM_CNT(eax, 0, 5) == 0) break;
                printf("%u ", u);
                switch (BITS_FROM_CNT(eax, 0, 5)) {
                    case 1 :  
                        printf("data       "); 
                        hw_info.cpuid_cache[u].type = 'D';
                        break;
                    case 2 :  
                        printf("instruction"); 
                        hw_info.cpuid_cache[u].type = 'I';
                        break;
                    case 3 :  
                        printf("unified    "); 
                        hw_info.cpuid_cache[u].type = 'U';
                        break;
                    default : 
                        printf("unknown    "); 
                }
                hw_info.cpuid_cache[u].level = BITS_FROM_CNT(eax, 5, 3);
                printf(" L%u", hw_info.cpuid_cache[u].level);

                hw_info.cpuid_cache[u].shared_by = BITS_FROM_CNT(eax, 26, 6)+1;
                printf(" shared by %u threads", hw_info.cpuid_cache[u].shared_by);

                way = BITS_FROM_CNT(ebx, 22, 10)+1;
                partition = BITS_FROM_CNT(ebx, 12, 10)+1;
                hw_info.cpuid_cache[u].line_size = BITS_FROM_CNT(ebx, 0, 12)+1;
                set = ecx+1;
                hw_info.cpuid_cache[u].size = way*partition*hw_info.cpuid_cache[u].line_size*set;

                printf(" %u-way, line:%u, size:%ukB", way, 
                        hw_info.cpuid_cache[u].line_size, 
                        hw_info.cpuid_cache[u].size);
                printf("\n");
                
               
                if (u>=MAX_CACHE) break;     // security break
                u++;
            }


        //} else if (hw_info.cpuid_max >= 0x02) {
        //    /* use Function 2 */
        } else {
            printf("no cache info for Intel found\n");
        }

    } else if (hw_info.cpu_vendor == vend_amd) {
        inline unsigned  amd_l23_assoc(unsigned code) {
            switch (code) {
                case 0 : return 0;
                case 1 : return 1;
                case 2 : return 2;
                case 4 : return 4;
                case 6 : return 8;
                case 8 : return 16;
                case 10 : return 32;
                case 11 : return 48;
                case 12 : return 64;
                case 13 : return 96;
                case 14 : return 128;
                case 15 : return 255;
            }
            return 0;
        }
        if (hw_info.cpuid_high_max >= 0x80000006) {
            /* use Function 0x8000_0005 for L1$ */
            cpuid(0x80000005);
            hw_info.cpuid_cache[0].type = 'I';
            hw_info.cpuid_cache[0].level = 1;
            hw_info.cpuid_cache[0].size = BITS_FROM_TO(edx, 24, 31) * 1024;
            hw_info.cpuid_cache[0].line_size = BITS_FROM_CNT(edx, 0, 8);
            printf("L1 instr. %u-way %u kB (line size: %u)\n", 
                    BITS_FROM_CNT(edx, 16, 8), 
                    hw_info.cpuid_cache[0].size/1024, 
                    hw_info.cpuid_cache[0].line_size);
            hw_info.cpuid_cache[1].type = 'D';
            hw_info.cpuid_cache[1].level = 1;
            hw_info.cpuid_cache[1].size = BITS_FROM_TO(ecx, 24, 31) *1024;
            hw_info.cpuid_cache[1].line_size = BITS_FROM_TO(ecx, 0, 7);
            printf("L1 data   %u-way %u kB (line size: %u)\n", 
                    BITS_FROM_CNT(ecx, 16, 8), 
                    hw_info.cpuid_cache[1].size/1024, 
                    hw_info.cpuid_cache[1].line_size);
            // TODO : eax, ebx contain TLB information
            
            cpuid(0x80000006);
            hw_info.cpuid_cache[2].size = BITS_FROM_TO(ecx, 16, 31) * 1024;
            if (hw_info.cpuid_cache[2].size > 0) {
                hw_info.cpuid_cache[2].type = 'U';
                hw_info.cpuid_cache[2].level = 2;
                hw_info.cpuid_cache[2].line_size = BITS_FROM_TO(ecx, 0, 7);
                printf("L2        %u-way %u kB (line size: %u)\n", 
                        amd_l23_assoc(BITS_FROM_CNT(ecx, 12, 4)), 
                        hw_info.cpuid_cache[2].size / 1024, 
                        hw_info.cpuid_cache[2].line_size);
            }

            hw_info.cpuid_cache[3].size = BITS_FROM_TO(edx, 18, 31)*512*1024;
            if (hw_info.cpuid_cache[3].size > 0) {
                hw_info.cpuid_cache[3].type = 'U';
                hw_info.cpuid_cache[3].level = 3;
                hw_info.cpuid_cache[3].line_size = BITS_FROM_TO(edx, 0, 7);
                printf("L3        %u-way %u kB (line size: %u)\n", 
                        amd_l23_assoc(BITS_FROM_CNT(edx, 12, 4)), 
                        hw_info.cpuid_cache[3].size/1024,
                        hw_info.cpuid_cache[3].line_size);
            }

        } else {
            printf("no cache info for AMD found\n");
        }

    }

    *pStatus = 0x0F00 + ' ';

}



/*
 * Functions for ACPI tables  ================================================================================
 */

static ptr_t search_rsdp(ptr_t offset, ptr_t size) 
{
    union {
        char str[9];
        uint32_t u32[2];
    } __attribute__((packed)) acpi_sig = { "RSD PTR " };

    volatile const uint32_t *p;
    unsigned i;

    p = (uint32_t*)offset;

    for (i = 0; i < size/sizeof(uint32_t); i++) {
        if (p[i] == acpi_sig.u32[0] && p[i+1] == acpi_sig.u32[1]) {
            return (ptr_t)(void*)&p[i];
        }
    }

    return 0;
}

/* check_hdr()
 *  - checksum
 *  - revision
 *  - optionally display OEMID, ...
 */
static int check_sum(void *hdr, size_t length)
{
    uint8_t sum = 0;
    uint8_t *p = (uint8_t*)hdr;
    unsigned i;

    for (i=0; i<length; i++) {
        sum += p[i];
    }
    if (sum != 0) {
        printf("WARNING: checksum = %hhu (should be 0)\n", sum);
    }
    return sum;
}

/*
 * The MADT (Multiprocessor APIC Description Table), Signature APIC,
 * contains information about the number of enabled processors and
 * the I/O APICs.
 */
static int read_madt(ptr_t offset)
{
    madt_t *madt = (madt_t*)offset;

    if (check_sum(madt, madt->header.length) != 0) {
        printf("WARNING: checksum of MADT invalid.\n");
        return -1;
    }

    hw_info.lapic_adr = madt->lapic_adr;
    IFV printf("local APIC adr: 0x%x\n", (ptr_t)hw_info.lapic_adr);

    unsigned i = 0;
    hw_info.cpu_cnt = 0;
    hw_info.ioapic_cnt = 0;
    while (__builtin_offsetof(madt_t, apic_structs)+i < madt->header.length) {
        unsigned subtype = madt->apic_structs[i];   /* first byte is type of entry */
        unsigned sublen = madt->apic_structs[i+1];  /* second byte is size of entry */
        madt_lapic_t *lapic;
        madt_ioapic_t *ioapic;

        switch (subtype) {
            case MADT_TYPE_LAPIC :
                lapic = (madt_lapic_t*)(ptr_t)&madt->apic_structs[i];
                if (lapic->flags.enabled) {
                    hw_info.cpu[hw_info.cpu_cnt].lapic_id = lapic->apic_id;
                    hw_info.cpu_cnt++;
                }
                IFV printf("CPU id=%u  enabled: %u local APIC id: %u\n", 
                        (ptr_t)lapic->acpi_processor_id, (ptr_t)lapic->flags.enabled, (ptr_t)lapic->apic_id);
                break;
            case MADT_TYPE_IOAPIC :
                ioapic = (madt_ioapic_t*)(ptr_t)&madt->apic_structs[i];
                hw_info.ioapic[hw_info.ioapic_cnt].id = ioapic->ioapic_id;
                hw_info.ioapic[hw_info.ioapic_cnt].adr = ioapic->ioapic_adr;
                hw_info.ioapic_cnt++;
                IFV printf("I/O APIC id=%u  adr: 0x%x\n", (ptr_t)ioapic->ioapic_id, (ptr_t)ioapic->ioapic_adr);
                break;

        }
        i += sublen;
    }

    return 0;
}

static int read_mcfg(ptr_t offset)
{
    unsigned i;
    mcfg_t *p_mcfg = (mcfg_t*)offset;
    check_sum(p_mcfg, p_mcfg->header.length);

    /* some static information */
    IFVV printf("MCFG Header Signature: %c%c%c%c len %u \n", 
            (p_mcfg->header.signature >> 0) & 0xFF,
            (p_mcfg->header.signature >> 8) & 0xFF,
            (p_mcfg->header.signature >> 16) & 0xFF,
            (p_mcfg->header.signature >> 24) & 0xFF,
            p_mcfg->header.length
            );

    for (i=0; i<(p_mcfg->header.length-44)/16; i++) {
        if ((ptr_t)((void*)&(p_mcfg->entry[i])-(void*)p_mcfg) > p_mcfg->header.length) break;
        IFVV printf("MCFG[%u] base 0x%x pci segm %u bus: %u-%u\n", i, 
                p_mcfg->entry[i].base_adr, 
                p_mcfg->entry[i].grp_nbr,
                p_mcfg->entry[i].bus_start,
                p_mcfg->entry[i].bus_end);
        if (i >= MAX_PCIE) {
            printf("ERROR: more PCI Express config spaces than supported!");
            halt();
        }
        hw_info.pcie_cfg[hw_info.pcie_cnt].base_adr  = p_mcfg->entry[i].base_adr;
        hw_info.pcie_cfg[hw_info.pcie_cnt].grp_nbr   = p_mcfg->entry[i].grp_nbr;
        hw_info.pcie_cfg[hw_info.pcie_cnt].bus_start = p_mcfg->entry[i].bus_start;
        hw_info.pcie_cfg[hw_info.pcie_cnt].bus_end   = p_mcfg->entry[i].bus_end;
        hw_info.pcie_cnt++;
    }
    return 0;
}

/*
 * functions for Multiprocessor Specification ================================================================
 */

static ptr_t search_fp(ptr_t offset, ptr_t size) 
{
    union {
        char str[5];
        uint32_t u32;
    } __attribute__((packed)) fp_sig = { "_MP_" };

    volatile const uint32_t *p;
    unsigned i;

    p = (uint32_t*)offset;

    for (i = 0; i < size/sizeof(uint32_t); i++) {
        if (p[i] == fp_sig.u32) {
            return (ptr_t)(void*)&p[i];
        }
    }

    return 0;
}







/* ===========================================================================================================
 * get_info()
 *
 * This function is called from the 32 bit startup code in real-mode to collect
 * information about the hardware from
 *  - BIOS Data Area (BDA) / Extended BDA (EBDA)
 *  - ACPI (if available) [1]
 *  - Multiprocessor Specification tables (if available)
 * The collected information is stored in internal data structures that are
 * shared with the protected mode kernel (32- or 64-bit).
 *
 *
 *  [1] Note: This OS does not support ACPI Power Management but only reads
 *      information about available CPUs and APICs from the ACPI tables.
 */

void get_info()
{
	//unsigned long addr;
	unsigned i;//, count;

    init_video();
    IFVV printf("get_info()\n");

    /* initialize hw_info to all 0's (but first uint32_t is left unchanged) */
    memset((void*)(&hw_info)+sizeof(uint32_t), 0, sizeof(hw_info)-sizeof(uint32_t));

    /* initialize some fields of hw_info */
    hw_info.cmd_cpumask = 0xFFFFFFFF;      /* default: all CPUs */
    hw_info.cmd_maxcpu = MAX_CPU;
    hw_info.tsc_per_usec = TSC_PER_USEC;            /* for 2.6 GHz CPU, used before calibration (see pit.c) */

    /*
     * check, what CPUID functions are available
     */
    cpu_features();


    /*
     * parse Multiboot's cmdline
     */
    multiboot_info_t *p_mbi = (multiboot_info_t*)hw_info.mb_adr;
    if (p_mbi->flags & (1<<2)) {
        char *cmdline = (char*)p_mbi->cmdline;

        while (cmdline != 0 && cmdline < (char*)p_mbi->cmdline+1000) {
            while (*cmdline != ' ' && *cmdline != 0) cmdline++;
            //printf("cmdline = '%s'\n", cmdline);
            if (strncmp(cmdline, " maxcpu=", 8) == 0) {
                cmdline += 8;
                hw_info.cmd_maxcpu = atoi(cmdline);
            } else if (strncmp(cmdline, " cpumask=", 9) == 0) {
                cmdline += 9;
                hw_info.cmd_cpumask = atoi(cmdline);
            } else if (strncmp(cmdline, " noacpi ", 8) == 0) {
                cmdline += 7;
                hw_info.cmd_noacpi = 1;
            } else if (strncmp(cmdline, " nomps ", 7) == 0) {
                cmdline += 6;
                hw_info.cmd_nomps = 1;
            }
            cmdline++;
        }
    }

    /*
     * first, read BIOS Data Area
     */
    bda_t *bda = (bda_t*)0x400;

    /* get EBDA Segment 16-bit real-mode segment, i.e. shift left by 4 bits to get physical address */
    ebda_t *ebda = (ebda_t*)((ptr_t)bda->segm_ebda << 4);
    hw_info.ebda_adr = bda->segm_ebda << 4;
    hw_info.ebda_size = ebda->size;
    
    /*
     * search for ACPI tables 
     *  - EBDA
     *  - physical address range 0xE0000 .. 0xFFFFF
     */

    if (hw_info.cmd_noacpi) goto skip_acpi;

    rsdp_t *rsdp = (rsdp_t*)search_rsdp(hw_info.ebda_adr, hw_info.ebda_size);
    if (rsdp == 0) {
        rsdp = (rsdp_t*)search_rsdp(0xE0000, 0x20000);
    }
    if (rsdp == 0) {
        printf("no RSDP found.\n");
        goto skip_acpi;
    }

    if (check_sum(rsdp, 20) != 0) {
        printf("WARNING: checksum of ACPI RSDP is incorrect.");
        goto skip_acpi;
    }

    rsdt_t *rsdt = (rsdt_t*)(ptr_t)rsdp->rsdt_adr;

    if (check_sum(rsdt, rsdt->header.length) != 0) {
        printf("WARNING: checksum of ACPI RSDT is incorrect.");
        goto skip_acpi;
    }

    for (i=0; i<(rsdt->header.length-sizeof(desc_hdr_t))/sizeof(uint32_t); i++) {
        desc_hdr_t *hdr = (desc_hdr_t*)(ptr_t)rsdt->entry[i];

        IFVV {
            char str[5] = { 0 };
            unsigned len = hdr->length;
            memcpy(str, &hdr->signature, 4);
            printf("RSDT Entry[%u]: 0x%x %s (size: %u)\n", i, rsdt->entry[i], str, len);
        }

        /* signature to table translation: see APICspec40a.pdf, Table 5-5, p. 114 */
        switch (hdr->signature) {
            //case FADT_SIGNATURE :  /* FACP -> FADT  */
                //printf("Fixed ACPI Descr. Table (FADT)\n");
                //break;
            case MADT_SIGNATURE :  /* APIC -> MADT  */
                IFVV printf("Multiple APIC Descr. T. (MADT)\n");
                read_madt(rsdt->entry[i]);
                break;
            case MCFG_SIGNATURE :
                IFVV printf("PCIe configuration table (MCFG)\n");
                read_mcfg(rsdt->entry[i]);
            //default :
                //printf("not supported, yet\n");

        };
    }
    IFVV {
        printf("halt to read output...\n");
        halt();
    }
    

    if (hw_info.lapic_adr != 0) {
        /* we have, what we need (skip multiprocessor specification tables) */
        goto skip_mps;
    }
        

skip_acpi:
    __asm__ ("nop");        /* instruction needed for label */

/*
 * now read multiprocessor tables
 */
    if (hw_info.cmd_nomps) goto skip_mps;

    /* 1. In the first kilobyte of the Extended BIOS Data Area (EBDA) */
    mps_fp_t *fp;
    fp = (mps_fp_t*)search_fp(hw_info.ebda_adr, 1024);
    /* 2. Within the last kilobyte of system base memory if EBDA is undefined */
    if (fp == 0) {
        fp = (mps_fp_t*)search_fp(639*1024, 1024);
    }
    /* 3. At the top of system physical memory */
    // ?!?
    
    /* 4. In the BIOS read-only memory space between 0xE0000 and 0xFFFFF */
    if (fp == 0) {
        fp = (mps_fp_t*)search_fp(0xE0000, 0x20000);
    }
    if (fp == 0) {
        printf("no FP found.\n");
        goto skip_mps;
    }

    if (check_sum(fp, fp->length*16) != 0) {
        printf("WARNING: checksum of MPS FP is incorrect.\n");
        goto skip_mps;
    }

    if (fp->spec_rev != 4 || fp->feature[0] != 0 || fp->offset_config == 0) {
        printf("WARNING: MPS revision or features not supported.\n");
        goto skip_mps;
    }

    mps_config_t *config = (mps_config_t*)(ptr_t)fp->offset_config;
    IFVV printf("config_t = 0x%x\n", config);

    if (check_sum(config, config->base_length) != 0) {
        printf("WARNING: checksum of MPS base CONFIG is incorrect.\n");
        goto skip_mps;
    }

    hw_info.lapic_adr = config->adr_lapic;

    if (check_sum(config, config->extd_len) != 0) {
        printf("WARNING: checksum of MPS extended CONFIG is incorrect.\n");
        goto skip_mps;
    }

    hw_info.cpu_cnt = 0;
    hw_info.ioapic_cnt = 0;
    uint8_t *type = (uint8_t*)((ptr_t)config + sizeof(mps_config_t));
    for (i=0; i < config->cnt_oem; i++) {
        mps_conf_processor_t *processor;
        mps_conf_ioapic_t *ioapic;
        switch (*type) {
            case 0 :
                processor = (mps_conf_processor_t*)type;
                IFVV printf("CPU %u\n", processor->lapic_id);
                if (processor->flags.en) {
                    hw_info.cpu[hw_info.cpu_cnt].lapic_id = processor->lapic_id;
                    hw_info.cpu_cnt++;
                }
                type += sizeof(mps_conf_processor_t);
                break;
            case 2 :
                ioapic = (mps_conf_ioapic_t*)type;
                IFVV printf("I/O APIC %u  0x%x\n", ioapic->ioapic_id, ioapic->adr_ioapic);
                if (ioapic->flags.en) {
                    hw_info.ioapic[hw_info.ioapic_cnt].id = ioapic->ioapic_id;
                    hw_info.ioapic[hw_info.ioapic_cnt].adr = ioapic->adr_ioapic;
                    hw_info.ioapic_cnt++;
                }
                type += sizeof(mps_conf_ioapic_t);
                break;
            default :
                type += 8;  /* all entries except processor are 8 bytes long */

        }
    }

skip_mps:

    if (hw_info.lapic_adr == 0 || hw_info.cpu_cnt == 0 || hw_info.ioapic_cnt == 0) {
        printf("ERROR: neither ACPI nor MP gave me information about CPUs and APICs. Halt.\n");
        //halt();
        hw_info.cpu_cnt = 1;
        hw_info.cpu[0].lapic_id = 0;
        hw_info.lapic_adr = 0xfee00000; /* just guessed (should be in MSR... */
        hw_info.ioapic_cnt = 0;
        /* so, this should work at least in single processor mode... */

    }
    IFV printf("found %d CPUs and %d I/O APICs\n", (ptr_t)hw_info.cpu_cnt, (ptr_t)hw_info.ioapic_cnt);


    return;
}
