/*
 * =====================================================================================
 *
 *       Filename:  nic.c
 *
 *    Description:  Network Interface Card drivers
 *
 *        Version:  1.0
 *        Created:  11.01.2012 13:27:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "pci.h"
#include "system.h"
#include "cpu.h"
#include "config.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_DRIVER > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_DRIVER > 1)

int rtl8139_init(unsigned bus, unsigned slot)
{
    uint16_t iobase;
    iobase = pci_config_read(bus, slot, 0, 0x10);
    if (iobase & 1) {
        // io-port: ignore 2 least significant bits
        iobase &= ~0x03;
    }
    IFV printf("rtl8139: iobase = 0x%x\n", iobase);

    uint8_t mac[6];
    unsigned u;
    for (u=0; u<6; u++) {
        mac[u] = inportb(iobase + u);
    }
    IFV printf("rtl8139: mac = %x:%x:%x:%x:%x:%x\n", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 1;  // the driver was not loaded, so far, we just print some information
}

