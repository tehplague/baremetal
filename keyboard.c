/*
 * =====================================================================================
 *
 *       Filename:  keyboard.c
 *
 *    Description:  keyboard driver (PS/2)
 *
 *        Version:  1.0
 *        Created:  11.01.2012 15:09:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "keyboard.h"
#include "system.h"
#include "cpu.h"
#include "lib.h"
#include "config.h"


#define IFV   if (VERBOSE > 0 || VERBOSE_KEYBOARD > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_KEYBOARD > 1)



/*
 * http://lowlevel.eu/wiki/KBC
 */

#define KBC_STATUS   0x64
#define KBC_EA       0x60

/*
 * send command to keyboard
 */
static void send_command(uint8_t command)
{
    // wait until cmd buffer is free
    while ((inportb(KBC_STATUS) & 2)) {}
    outportb(KBC_EA, command);
}

/*
 * get scan code (can be called to poll or by interrupt handler)
 * polling: should be done until the buffer is empty
 * interrupt: should be called once and afterwards signal EOI!
 */
uint32_t keyboard_get_scancode()
{
    static unsigned e0_code = 0;
    static unsigned e1_code = 0;
    static uint16_t e1_prev = 0;

    uint8_t scancode = 0;
   
    if (inportb(KBC_STATUS) & 1) {
        // a scancode is available in the buffer
        scancode = inportb(KBC_EA);
        if (e0_code == 1) {
            // scancode is an e0 code
            
            e0_code = 0;
            return (0xe000 | scancode);
        } else if (e1_code == 1) {
            // scancode is first byte of e1 code
            e1_prev = scancode;
            e1_code = 2;
        } else  if (e1_code == 2) {
            // scancode is second byte of e1 code (first is in e1_prev)

            e1_code = 0;
            return (0xe10000 | e1_prev << 8 | scancode);
        } else if (scancode == 0xe0) {
            e0_code = 1;
            scancode = 0;
        } else if (scancode == 0xe1) {
            e1_code = 1;
            scancode = 0;
        }
    }

    return scancode;
}

#define KBM_SHIFT   0
#define KBM_CTRL    1
#define KBM_ALT     2
#define KBM_ALT_GR  3
static uint32_t kb_modifier = 0;
/*
 * see: http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
 */
uint8_t scancode_to_keycode(uint32_t scancode)
{
    switch (scancode) {
        case 0x2a : BIT_SET(kb_modifier, KBM_SHIFT); break; 
        case 0xaa : BIT_CLEAR(kb_modifier, KBM_SHIFT); break; 

        case 0x36 : BIT_SET(kb_modifier, KBM_SHIFT); break; 
        case 0xb6 : BIT_CLEAR(kb_modifier, KBM_SHIFT); break; 

        case 0x1d : BIT_SET(kb_modifier, KBM_CTRL); break; 
        case 0x9d : BIT_CLEAR(kb_modifier, KBM_CTRL); break; 

        case 0x01 : return KEY_ESC;
        case 0x02 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '1' : '!'; break;
        case 0x03 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '2' : '"'; break;
        case 0x04 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '3' : '3'; break; // compiler says: '§' is a multibyte character...
        case 0x05 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '4' : '$'; break;
        case 0x06 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '5' : '%'; break;
        case 0x07 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '6' : '&'; break;
        case 0x08 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '7' : '/'; break;
        case 0x09 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '8' : '('; break;
        case 0x0a : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '9' : ')'; break;
        case 0x0b : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? '0' : '='; break;

        case 0x10 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'q' : 'Q'; break;
        case 0x11 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'w' : 'W'; break;
        case 0x12 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'e' : 'E'; break;
        case 0x13 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'r' : 'R'; break;
        case 0x14 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 't' : 'T'; break;
        case 0x15 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'z' : 'Z'; break;
        case 0x16 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'u' : 'U'; break;
        case 0x17 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'i' : 'I'; break;
        case 0x18 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'o' : 'O'; break;
        case 0x19 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'p' : 'P'; break;
        case 0x1c : return '\n'; break;

        case 0x1e : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'a' : 'A'; break;
        case 0x1f : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 's' : 'S'; break;
        case 0x20 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'd' : 'D'; break;
        case 0x21 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'f' : 'F'; break;
        case 0x22 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'g' : 'G'; break;
        case 0x23 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'h' : 'H'; break;
        case 0x24 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'j' : 'J'; break;
        case 0x25 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'k' : 'K'; break;
        case 0x26 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'l' : 'L'; break;

        case 0x2c : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'y' : 'Y'; break;
        case 0x2d : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'x' : 'X'; break;
        case 0x2e : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'c' : 'C'; break;
        case 0x2f : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'v' : 'V'; break;
        case 0x30 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'b' : 'B'; break;
        case 0x31 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'n' : 'N'; break;
        case 0x32 : return IS_BIT_CLEAR(kb_modifier, KBM_SHIFT) ? 'm' : 'M'; break;
        case 0x39 : return ' '; break;

        case 0xe048 : return KEY_UP; break;
        case 0xe050 : return KEY_DOWN; break;
        case 0xe049 : return KEY_PGUP; break;
        case 0xe051 : return KEY_PGDOWN; break;
        case 0xe047 : return KEY_HOME; break;
        case 0xe04f : return KEY_END; break;
    }
    return 0;
}

uint8_t wait_for_key()
{
    uint32_t sc;
    while ((sc = keyboard_get_scancode()) == 0)  {
        udelay(1000);
    }
    return scancode_to_keycode(sc);
}

int keyboard_mode(kb_mode_t mode)
{
    if (mode == kbm_poll) {
        /* remove keyboard interrupt handler */
        return 0;
    } else {
        /* install keyboard interrupt handler */
        printf("sorry, interrupt based keyboard mode not supported, yet.\n");
        return 1;
    }
    return 0;
}

int keyboard_init(kb_mode_t mode)
{
    unsigned char c;

    keyboard_mode(mode);
    
    // empty keyboard buffer
    while (inportb(KBC_STATUS) & 1) {
        inportb(KBC_EA);
    }   
 
    // activate keyboard
    send_command(0xF4);
    while (inportb(KBC_STATUS) & 1) {
        inportb(KBC_EA);    // read (and drop) what's left in the keyboard buffer
    }   

    // self-test (should answer with 0xEE)
    send_command(0xEE);
    c = 0;
    while (inportb(KBC_STATUS) & 1) {
        c = inportb(KBC_EA);
        IFV printf("keyboard self test: send 0xEE, received 0x%x (should be 0xEE)\n", (ptr_t)c);
    }   





    return 0;
}

