/*
 * =====================================================================================
 *
 *       Filename:  nic.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.01.2012 13:27:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef NIC_H
#define NIC_H

#include "stddef.h"

int rtl8139_init(unsigned bus, unsigned slot);

#endif // NIC_H

