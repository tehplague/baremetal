/*
 * =====================================================================================
 *
 *       Filename:  pci.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.12.2011 10:04:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef PCI_H
#define PCI_H

#include "stddef.h"

uint16_t pci_config_read(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset);
void pci_init();

#endif // PCI_H
