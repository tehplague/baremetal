/*
 * =====================================================================================
 *
 *       Filename:  bda.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08.09.2011 10:34:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef BDA_H
#define BDA_H

#include "stddef.h"

#define BDA_EQUIP_VIDEO_EGA     0   /* EGA or older */
#define BDA_EQUIP_VIDEO_COL40   1   /* Color 40x25  */
#define BDA_EQUIP_VIDEO_COL80   2   /* Color 80x25  */
#define BDA_EQUIP_VIDEO_MONO80  3   /* Monochrome 80x25 */

/*
 * BIOS Data Area: structure of values
 * see:  [1] http://lowlevel.eu/wiki/BIOS_Data_Area
 * BDA is always at physical address 0x400
 */
typedef struct {
    /* offset 0x00 */
    uint16_t io_com1;     /* base address of I/O Port for COM1 (1st serial port) */
    uint16_t io_com2;     /* dto COM2 */
    uint16_t io_com3;     /* dto COM3 */
    uint16_t io_com4;     /* dto COM4 */
    uint16_t io_lpt1;     /* base address of I/O Port for LPT1 (1st parallel port) */
    uint16_t io_lpt2;     /* dto LPT2 */
    uint16_t io_lpt3;     /* dto LPT3 */
    uint16_t segm_ebda;   /* dto LPT4 *OR* segment of EBDA (for PS/2) */
    /* offset 0x10 */
    struct {
        uint16_t has_bootfloppy : 1;  /* has bootfloppy drive */
        uint16_t has_fpu        : 1;  /* has mathematic coprocessor (FPU) */
        uint16_t has_ps2_mouse  : 1;  /* has PS/2 mouse */
        uint16_t reserved       : 1;  /* (reserved) */
        uint16_t video          : 2;  /* see BDA_EQUIP_VIDEO_XXX */
        uint16_t floppy         : 2;  /* 0: one floppy drive, 1: two drives */
        uint16_t has_dma        : 1;  /* has DMA controller */
        uint16_t cnt_com        : 3;  /* number of serial COM ports */
        uint16_t has_joystick   : 1;  /* has joystick */
        uint16_t has_modem      : 1;  /* has internal modem */
        uint16_t cnt_lpt        : 2;  /* number of parallel LPT ports */
    } equipment;
    uint8_t post;           /* POST status */
    uint16_t mem_conv;      /* size of conventional memory */
    /* TODO: further fields not implemented, yet (but contain no more reliable information,
     * according to some web pages) */
    uint8_t dummy1[11];   
    /* offset 0x20 */
    uint16_t dummy2[14*8];
} __attribute__((packed)) bda_t;


/*
 * Extended BIOS Data Area EBDA
 * see: [2] http://www.scribd.com/doc/52627856/43/Extended-BIOS-Data-Area
 * see: [3] http://web.archive.org/web/20060508100419/http://heim.ifi.uio.no/~stanisls/helppc/ebda.html
 * This area is located in memory just below 640k. The exact position can be obtained from BDA (0x40E).
 *
 * BDA contains Sement Address [1]
 * INT 15h AC=C1h returns the segment starting address of this table [2]
 *
 */
typedef struct {
    uint8_t size;           /* size of EBDA in kB */
    uint8_t reserved[33];   /*  */
    /* TODO: further fields not implemented, yet */

} __attribute__((packed)) ebda_t;


extern ptr_t offset_ebda;
extern ptr_t size_ebda;


#endif // BDA_H
