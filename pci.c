/*
 * =====================================================================================
 *
 *       Filename:  pci.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.12.2011 10:03:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "cpu.h"
#include "sync.h"
#include "info.h"
#include "driver.h"


#define IFV   if (VERBOSE > 0 || VERBOSE_PCI > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_PCI > 1)

#define CONFIG_ADR  0xCF8
#define CONFIG_DATA 0xCFC

mutex_t pci_mutex = MUTEX_INITIALIZER;

/*  
 *  see: http://wiki.osdev.org/PCI
 */

uint16_t pci_config_read(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
    uint32_t adr;
    uint16_t value;

    adr = 0x80000000ul | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | (func << 8) | (offset & 0xfc);
    mutex_lock(&pci_mutex);
    outportl(CONFIG_ADR, adr);
    //udelay(100);
    value = (uint16_t)((inportl(CONFIG_DATA) >> ((offset & 2)*8)) & 0xFFFF);
    mutex_unlock(&pci_mutex);
    return value;
}

static unsigned pcie_configured = 0;
uint32_t pcie_config_read(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
    ptr_t adr;
    uint32_t *p;

    if (!pcie_configured) {
        printf("ERROR: PCIE not (yet) configured (memory not mapped).\n");
        halt();
    }

    adr = hw_info.pcie_cfg[0].base_adr 
        + (((ptr_t)bus << 20) | ((ptr_t)slot << 15) | ((ptr_t)slot << 15) | ((ptr_t)func << 12) | (offset & 0xFFF));

    p = (uint32_t*)adr;
    return *p;
}

void pci_init()
{
    unsigned bus, slot;
    uint16_t vendor, device;
    unsigned u;

    IFV printf("pci_init()\n");

    /*
     * read PCI devices
     */
    for (bus=0; bus<16; bus++) {
        for (slot=0; slot<32; slot++) {
            vendor = pci_config_read(bus, slot, 0, 0);
            if (vendor != 0xFFFF) {
                device = pci_config_read(bus, slot, 0, 2);
                IFV printf("PCI: %2x:%2x  [%4x:%4x]\n", bus, slot, vendor, device);
                driver_check_pci(vendor, device, bus, slot);
            }
        }
    }

    /*
     * map PCIe configuration space
     */
    if (hw_info.pcie_cnt > 0) {
        // as long as PCIE_CNT == 1, we don't need to search for the config space, as there is only one.

        for (u=0; u<hw_info.pcie_cnt; u++) {
            IFVV printf("PCIe[%u]: 0x%x bus %u-%u\n", u, hw_info.pcie_cfg[u].base_adr, 
                    (unsigned)hw_info.pcie_cfg[u].bus_start, (unsigned)hw_info.pcie_cfg[u].bus_end);
            
            // TODO: where to map this?
            //       -> QEMU does not have any PCIe regions...

        }

        //pcie_configured = 1;

        /*
         * read PCIe devices
         */
        if (pcie_configured) {
            for (bus=0; bus<2; bus++) {
                for (slot=0; slot<64; slot++) {
                    //vendor = pci_config_read(bus, slot, 0, 0);
                    vendor = pcie_config_read(bus, slot, 0, 0);
                    if (vendor != 0xFFFF) {
                        device = pci_config_read(bus, slot, 0, 2);
                        IFV printf("PCIe: %2x:%2x  [%4x:%4x]\n", bus, slot, vendor, device);
                    }
                }
            }
        }
    } else {
        IFVV printf("PCIe: not found in ACPI tables.\n");
    }

    IFV printf("pci_init() finished.\n");
}

