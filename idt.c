#include "system.h"

/* Defines an IDT entry */
#ifdef __x86_64__

struct idt_entry
{
    uint16_t base_lo;
    uint16_t sel;        /* Our kernel segment goes here! */
    uint8_t  ist;         /* 3 bits IST, above only 0s     */
    uint8_t  flags;       /* Set using the above table!    */
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t dummy;
} __attribute__((packed));
#else
struct idt_entry
{
    uint16_t base_lo;
    uint16_t sel;        /* Our kernel segment goes here! */
    uint8_t  always0;     /* This will ALWAYS be set to 0! */
    uint8_t  flags;       /* Set using the above table! */
    uint16_t base_hi;
} __attribute__((packed));
#endif

struct idt_ptr
{
    unsigned short limit;
    void *base;
} __attribute__((packed));


/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* This exists in 'jump64.asm', and is used to load our IDT */
extern void idt_load();

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    /* We'll leave you to try and code this function: take the
    *  argument 'base' and split it up into a high and low 16-bits,
    *  storing them in idt[num].base_hi and base_lo. The rest of the
    *  fields that you must set in idt[num] are fairly self-
    *  explanatory when it comes to setup */
#if __x86_64__
    idt[num].base_lo = base & 0xFFFF;
    idt[num].sel = sel; //.Code;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
#else
    idt[num].base_lo = base & 0xFFFF;
    idt[num].sel = sel; //.Code;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].base_hi = base >> 16;
#endif
}

/* Installs the IDT */
void idt_install()
{
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = &idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    /* Add any new ISRs to the IDT here using idt_set_gate */

    /* Points the processor's internal register to the new IDT */
    idt_load();
}

/* Installs the IDT on the Application Processors */
void idt_install_ap()
{
    /* Points the processor's internal register to the new IDT */
    idt_load();
}
