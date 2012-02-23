/*
 * =====================================================================================
 *
 *       Filename:  acpi_intern.h
 *
 *    Description:  internal structures (only for acpi.c)
 *
 *        Version:  1.0
 *        Created:  09.09.2011 12:33:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef ACPI_INTERN_H
#define ACPI_INTERN_H

#include "stddef.h"

/*
 * RSDP - Root System Descption Pointer
 * (ACPIspec40a.pdf, p. 112)
 */
typedef struct {
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_adr;
    uint32_t length;
    uint64_t xsdt_adr;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

/*
 * System Description Header
 * (ACPIspec40a.pdf, p. 113)
 * 
 * The following tables begin with this header.
 */
typedef struct {
    uint32_t signature;                   /* see: ACPIspec40a.pdf, Table 5-5 and 5-6, p. 114 */
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oem_table_id;
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creater_revision;
} __attribute__((packed)) desc_hdr_t;

/*
 * RSDT - Root System Description Table
 * (ACPIspec40a.pdf, p. 116)
 */
typedef struct {
    desc_hdr_t header;               /* signature: RSDT */
    uint32_t entry[];                /* number of entries is calculated from header.length */
} __attribute__((packed)) rsdt_t;

/*
 * XSDT - Extended System Description Table
 * (ACPIspec40a.pdf, p. 117)
 */
typedef struct {
    desc_hdr_t header;                  /* signature: XSDT */
    uint64_t entry[];                   /* number of entries is calculated from header.length */
} __attribute__((packed)) xsdt_t;


/*
 * FADT - Fixed ACPI Description Table
 * (ACPIspec40a.pdf, p. 118)
 */
#define FADT_SIGNATURE  ('F'|'A'<<8|'C'<<16|'P'<<24)
typedef struct {
    desc_hdr_t header;                  /* signature: FACP */
    uint32_t facs_adr;                  /* -> FACS */
    uint32_t dsdt_adr;                  /* -> DSDT */
    uint8_t int_model;                  /* revision 1.0; now "reserved" */
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
} __attribute__((packed)) fadt_t;

/*
static char *fadt_preferred_pm[] = {
    "Unspecified",
    "Desktop",
    "Mobile",
    "Workstation",
    "Enterprise Server",
    "SOHO Server",
    "Appliance PC",
    "Performance Server",
    [8 ... 25] = "Reserved"
};
*/


/*
 * MADT - Multiple APIC Description Table
 * (ACPIspec40a.pdf, p. 136)
 */
#define MADT_SIGNATURE  ('A'|'P'<<8|'I'<<16|'C'<<24)
typedef struct {
    desc_hdr_t header;                  /* signature: APIC */
    uint32_t lapic_adr;
    struct {
        uint32_t pcat_compat : 1;
        uint32_t reserved    : 31;
    } flags;
    uint8_t apic_structs[];             /* number of entries (they're of variable size...) must be derived from header.length  */
} __attribute__((packed)) madt_t;

#define MADT_TYPE_LAPIC     0
#define MADT_TYPE_IOAPIC    1
#define MADT_TYPE_INTSRC    2
#define MADT_TYPE_LAPIC_NMI 4

/*
static char *madt_type[] = {
    "Processor Local APIC",
    "I/O APIC",
    "Interrupt Source Override",
    "Non-maskable Interrupt Source (NMI)",
    "Local APIC NMI",
    "Local APIC Address Override",
    "I/O SAPIC",
    "Local SAPIC",
    "Platform interrupt Sources",
    "Processor Local x2APIC",
    "Local x2APIC NMI",
    [0xB ... 0x7F] = "Reserved",
    [0x80 ... 0xFF] = "Reserved"
};
*/

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_hdr_t;

typedef struct {
    madt_hdr_t header;
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    struct {
        uint32_t enabled     : 1;
        uint32_t reserved    : 31;
    } flags;
} __attribute__((packed)) madt_lapic_t;

typedef struct {
    madt_hdr_t header;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_adr;
    uint32_t gsib;
} __attribute__((packed)) madt_ioapic_t;

typedef struct {
        uint16_t polarity     :  2;
        uint16_t trigger_mode :  2;
        uint16_t reserved     : 12;
} mps_inti_flags;

typedef struct {
    madt_hdr_t header;
    uint8_t bus;
    uint8_t source;
    uint32_t gsi;
    mps_inti_flags flags;
} __attribute__((packed)) madt_intsrc_t;

typedef struct {
    madt_hdr_t header;
    uint8_t processor_id;
    mps_inti_flags flags;
    uint8_t lapic_lint;
} __attribute__((packed)) madt_lapic_nmi_t;


/*
 * MADT - Multiple APIC Description Table
 * (see: http://wiki.osdev.org/PCI_Express)
 */
#define MCFG_SIGNATURE  ('M'|'C'<<8|'F'<<16|'G'<<24)
typedef struct {
    desc_hdr_t header;                  /* signature: APIC */
    uint64_t reserved;
    struct {
        uint64_t base_adr;
        uint16_t grp_nbr;
        uint8_t bus_start;
        uint8_t bus_end;
        uint32_t reserved;
    } entry[];
} __attribute__((packed)) mcfg_t;

#endif // ACPI_INTERN_H

