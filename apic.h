/*
 * =====================================================================================
 *
 *       Filename:  apic.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  17.11.2011 21:02:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef APIC_H
#define APIC_H

void apic_eoi(void);
void send_ipi(uint8_t to, uint8_t vector);
void apic_init();
void apic_init_ap(unsigned id);

#endif // APIC_H
