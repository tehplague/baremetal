#include "system.h"
#include "sync.h"
#include "smp.h"
#include "cpu.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_APIC > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_APIC > 1)

#define LAPIC_REG_ID        0x0020
#define LAPIC_REG_VERSION   0x0030
#define LAPIC_REG_EOI       0x00B0
#define LAPIC_REG_SPURIOUS  0x00F0
#define LAPIC_ICR_LOW       0x0300
#define LAPIC_ICR_HIGH      0x0310
#define LAPIC_LVT_TIMER     0x0320
#define LAPIC_LVT_ERROR     0x0370

static ptr_t localAPIC =   0xfee00000UL;
static ptr_t ioAPIC_base = 0xfec00000UL;

volatile unsigned cpu_online = 0;

uint32_t read_localAPIC(uint32_t offset)
{
    volatile uint32_t *localAPIC_Register = (uint32_t*)(localAPIC+offset);
    return *localAPIC_Register;
}
void write_localAPIC(uint32_t offset, uint32_t value)
{
    volatile uint32_t *localAPIC_Register = (uint32_t*)(localAPIC+offset);
    //IFVV printf("write_localAPIC: 0x%x  -> 0x%x\n", (ptr_t)localAPIC_Register, value);
    *localAPIC_Register = value;
}
void set_localAPIC(uint32_t offset, uint32_t mask, uint32_t value)
{
    volatile uint32_t *localAPIC_Register = (uint32_t*)(localAPIC+offset);
    uint32_t reg = *localAPIC_Register;
    reg &= ~mask;
    reg |= value;
    *localAPIC_Register = reg;
}

uint32_t read_ioAPIC(unsigned id, uint32_t offset)
{
    // TODO : store IF, CLI
    volatile uint32_t* ioAPIC_Adr = (uint32_t*)(ioAPIC_base + (id*4096));
    volatile uint32_t* ioAPIC_Data = (uint32_t*)(ioAPIC_base + (id*4096) + 0x10);
    *ioAPIC_Adr = offset;
    /*  TODO: wait?! */
    return *ioAPIC_Data;
    // TODO : restore IF to previous value
}
void write_ioAPIC(unsigned id, uint32_t offset, uint32_t value)
{
    // TODO : store IF, CLI
    volatile uint32_t* ioAPIC_Adr = (uint32_t*)(ioAPIC_base + (id*4096));
    volatile uint32_t* ioAPIC_Data = (uint32_t*)(ioAPIC_base + (id*4096) + 0x10);
    *ioAPIC_Adr = offset;
    /*  TODO: wait?! */
    *ioAPIC_Data = value;
    // TODO : restore IF to previous value
}
#define ICR_LVL_ASSERT          (1u <<14)
#define ICR_DLV_STATUS          (1u <<12)
#define ICR_MODE_FIXED          (0u <<8)

void send_ipi(uint8_t to, uint8_t vector)
{
    uint32_t value;
    unsigned if_backup;
    if_backup = cli();
    // TODO: wait until APIC has processed a previous IPI (see: MetalSVM, arch/x86/kernel/apic.c:ipi_tlb_flush() )

    IFVV printf("send_ipi()  to: %u  lapic_id: %u  vector: %u\n", (unsigned long)to, (unsigned long)hw_info.cpu[to].lapic_id, (unsigned long)vector);
    value = read_localAPIC(LAPIC_ICR_HIGH);
    value &= 0x00FFFFFF;
    value |= ((uint32_t)hw_info.cpu[to].lapic_id << 24);
    write_localAPIC(LAPIC_ICR_HIGH, value);

    /*
    value = (0u << 18)   // Destination Shorthand: 00 - No Shorthand
            |(0u << 15)  // Trigger Mode: 0 - Edge
            |(1u << 14)  // Level: 1 - Assert (for all except INIT IPI)
            |(1u << 12)  // Delivery Status: 1 - Send Pending
            |(0u << 11)  // Destination Mode: 0 - Physical
            |(0u << 8)   // Delivery Mode: 000 - Fixed
            |vector;
            */
    value = ICR_LVL_ASSERT | ICR_DLV_STATUS | ICR_MODE_FIXED | vector;
    write_localAPIC(LAPIC_ICR_LOW, value);
    if (if_backup) sti();
}


void apic_eoi(void)
{
    write_localAPIC(LAPIC_REG_EOI, 0);
}

extern stack_t stack[MAX_CPU];

void apic_init()
{
    uint16_t u;
    /* the presence of a localAPIC (CPUID(EAX=1).EDX[bit 9]) was already checked in startXX.asm */

    IFVV printf("apic_init()\n");

    /* local APIC address is in hw_info */
    localAPIC = hw_info.lapic_adr;


    /* support only VERSION >= 0x10 */
    
    cli();  // no interrupts!

    /* deactivate PIC */
    outportb(0xA1, 0xFF);
    outportb(0x21, 0xFF);

    IFVV printf("apic_init: PIC deactivated.\n");

    /* 
     * activate local APIC 
     */

    /* Presence: CPUID.01h:EDX[bit 9] (checked already in start.asm) */

    /* To initialise the BSP's local APIC, set the enable bit in the spurious
     * interrupt vector register and set the error interrupt vector in the
     * local vector table.  */
    set_localAPIC(LAPIC_REG_SPURIOUS, (1<<8), (1<<8));

    IFVV printf("local APIC version: 0x%x  max LVT entry: %u\n", 
            read_localAPIC(LAPIC_REG_VERSION) && 0xFF, 
            ((read_localAPIC(LAPIC_REG_VERSION)>>16) && 0xFF)+1);

    /*
     * according to http://www.osdever.net/tutorials/view/multiprocessing-support-for-hobby-oses-explained
     * the spurios int vector can be ignored (use 0x1F for now...)
     * and the lowest 4 bits are hardwired to 1 (only 0x?F can be used)
     */
    write_localAPIC(LAPIC_LVT_ERROR, 0x1F);       /* 0x1F: temporary vector (all other bits: 0) */


    /* 
     * start APs 
     */

    /* install initial code to a physical page below 640 kB */

    uint8_t *ptr = (uint8_t*)(ptr_t)(SMP_FRAME << 12);

    extern uint8_t smp_start[];
    extern uint16_t smp_apid;
    extern uint8_t smp_end;
    uint16_t size = (uint16_t)((ptr_t)&smp_end - (ptr_t)&smp_start);

    /* pointer to the variable smp_apid in that page */
    volatile uint16_t *ptr_apid = (void*)ptr + ((ptr_t)&smp_apid - (ptr_t)&smp_start);

    IFVV printf("smp_start = 0x%x  smp_end = 0x%x  size = %u\n", (ptr_t)&smp_start, (ptr_t)&smp_end, size);

    if (size > PAGE_SIZE) {
        printf("WARNING: SMP start code larger than one page!\n");
        return; 
    }

    /* copy code byte-wise (TODO: why not use memcpy?) */
    for (u=0; u<size; u++) {
        ptr[u] = smp_start[u];
    }
    



    /* set up status monitor for APs */
    status_putch(5, '[');
    status_putch(6, STATUS_RUNNING);
    status_putch(6+hw_info.cpu_cnt, ']');

    /* now send IPIs to the APs */
    for (u = 1; u < hw_info.cpu_cnt; u++) {
        if (u >= hw_info.cmd_maxcpu || ((hw_info.cmd_cpumask & (1 << u)) == 0)) {
            status_putch(6+u, STATUS_NOTUP);
            IFV printf("SMP: skip AP#%u\n", u);
            continue;
        }
        status_putch(6+u, STATUS_WAKEUP);
        *ptr_apid = u;
        IFV printf("SMP: try to wake up AP#%u\n", u);
        IFVV printf("  #%u: send INIT IPI\n", u);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[u].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x5 << 8)|SMP_FRAME);

        udelay(10*1000); /* 10 ms */
        
        IFVV printf("  #%u: send first SIPI\n", u);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[u].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x6 << 8)|SMP_FRAME);

        udelay(200);  /* 200 us */
        
        IFVV printf("  #%u: send second SIPI\n", u);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[u].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x6 << 8)|SMP_FRAME);

        udelay(100 * 1000); /* 100 ms */
        if (mutex_trylock(&(stack[u].info.wakelock))) {
            /*
             * lock obtained successfully => AP did not lock it in time 
             * if we lock the wakelock, the AP will block in main_ap()
             */
            printf("  #%u: CPU did not come up.\n", u);
        }
        
    }

    udelay(100*1000);
    IFV printf("all %u APs called, %u up\n", hw_info.cpu_cnt-1, cpu_online);


    /* TODO: activate I/O APIC */


    /* TODO: calibrate TSC (rdtsc) and CLK (time-base of local APIC timer) */




}

void apic_init_ap(unsigned id)
{

    /* support only VERSION >= 0x10 */

    /* deactivate PIC */
    //outportb(0xA1, 0xFF);
    //outportb(0x21, 0xFF);

    /* 
     * activate local APIC 
     */

    /* Presence: CPUID.01h:EDX[bit 9] (checked already in start.asm) */

    /* To initialise the BSP's local APIC, set the enable bit in the spurious
     * interrupt vector register and set the error interrupt vector in the
     * local vector table.  */
    set_localAPIC(LAPIC_REG_SPURIOUS, (1<<8), (1<<8));

    /*
     * is this needed?!
     */
    write_localAPIC(LAPIC_REG_ID, (id&0x7F)<<24);

    IFVV printf("local APIC version: 0x%x  max LVT entry: %u\n", 
            read_localAPIC(LAPIC_REG_VERSION) && 0xFF, 
            ((read_localAPIC(LAPIC_REG_VERSION)>>16) && 0xFF)+1);

    /*
     * according to http://www.osdever.net/tutorials/view/multiprocessing-support-for-hobby-oses-explained
     * the spurios int vector can be ignored (use 0x1F for now...)
     * and the lowest 4 bits are hardwired to 1 (only 0x?F can be used)
     */
    write_localAPIC(LAPIC_LVT_ERROR, 0x1F);       /* 0x1F: temporary vector (all other bits: 0) */
}

