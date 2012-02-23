/*
 * =====================================================================================
 *
 *       Filename:  keyboard.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.01.2012 15:09:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stddef.h"

typedef enum {kbm_poll, kbm_interrupt} kb_mode_t;
#define KEY_ESC       200
#define KEY_UP        201
#define KEY_DOWN      202
#define KEY_PGUP      205
#define KEY_PGDOWN    206
#define KEY_HOME      207
#define KEY_END       208

uint32_t keyboard_get_scancode();
uint8_t scancode_to_keycode(uint32_t scancode);
uint8_t wait_for_key();
int keyboard_mode(kb_mode_t mode);
int keyboard_init(kb_mode_t mode);

#endif // KEYBOARD_H
