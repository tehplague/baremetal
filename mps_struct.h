/*
 * =====================================================================================
 *
 *       Filename:  mps_intern.h
 *
 *    Description:  definitions (only) for mps.c
 *
 *        Version:  1.0
 *        Created:  09.09.2011 21:25:08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef MPS_INTERN_H
#define MPS_INTERN_H

#include "stddef.h"

typedef struct {
    uint32_t sig;
    uint32_t offset_config;
    uint8_t length;
    uint8_t spec_rev;
    uint8_t checksum;
    uint8_t feature[5];
} __attribute__((packed)) mps_fp_t;

typedef struct {
    uint32_t sig;
    uint16_t base_length;
    uint8_t spec_rev;
    uint8_t checksum;
    uint8_t oem_id[8];
    uint8_t prod_id[12];
    uint32_t offset_oem;
    uint16_t size_oem;
    uint16_t cnt_oem;
    uint32_t adr_lapic;
    uint16_t extd_len;
    uint8_t extd_checksum;
    uint8_t reserved;
} __attribute__((packed)) mps_config_t;

typedef struct {
    uint8_t type;
    uint8_t lapic_id;
    uint8_t lapic_version;
    struct {
        uint8_t en : 1;
        uint8_t bp : 1;
        uint8_t reserved : 6;
    } flags;
    uint32_t cpu_signature;
    uint32_t cpu_features;
    uint32_t reserved[2];
} __attribute__((packed)) mps_conf_processor_t;

typedef struct {
    uint8_t type;
    uint8_t bus_id;
    uint8_t bus_type[6];
} __attribute__((packed)) mps_conf_bus_t;

typedef struct {
    uint8_t type;
    uint8_t ioapic_id;
    uint8_t ioapic_version;
    struct {
        uint8_t en : 1;
        uint8_t reserved : 7;
    } flags;
    uint32_t adr_ioapic;
} __attribute__((packed)) mps_conf_ioapic_t;

typedef struct {
    uint8_t type;
    uint8_t int_type;
    struct {
        uint16_t po : 2;
        uint16_t el : 2;
        uint8_t reserved : 7;
    } flags;
    uint8_t src_bus_id;
    uint8_t src_bus_irq;
    uint8_t dest_apic_id;
    uint8_t dest_apic_intin;
} __attribute__((packed)) mps_conf_int_t;


#endif // MPS_INTERN_H
