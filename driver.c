/*
 * =====================================================================================
 *
 *       Filename:  driver.c
 *
 *    Description:  interface for device drivers
 *
 *        Version:  1.0
 *        Created:  11.01.2012 10:23:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */


#include "config.h"
#include "system.h"
#include "pci.h"
#include "stddef.h"
#include "nic.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_DRIVER > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_DRIVER > 1)

int driver_check_pci(uint16_t vendor, uint16_t device, unsigned bus, unsigned slot)
{
    uint32_t vendev = (uint32_t)vendor << 16 | device;
    switch (vendev) {
        case 0x10ec8139 :
            // found in qemu (32 and 64)
            IFV printf("found: Realtek RTL 8139 Fast Ethernet NIC (%x:%x)\n", bus, slot);
            rtl8139_init(bus, slot);
            return 1;
            break;
        case 0x808610cc :
            // found on xaxis
            IFV printf("found: Intel Ethernet NIC (%x:%x)\n", bus, slot);
            // call driver_init
            return 1;
            break;
        default:
            IFVV printf("unknown vendor/device.\n");
    }
    return 0;
}
