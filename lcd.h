/*
 * Functions for using the RP2040-GEEK LCD from the emulation
 *
 * Copyright (C) 2024 by Udo Munk & Thomas Eberhardt
 */

#ifndef LCD_INC
#define LCD_INC

#include "LCD_1in14_V2.h"
#include "GUI_Paint.h"

extern void lcd_init(void), lcd_exit(void);
extern void lcd_set_rotated(int rotated);
extern void lcd_set_draw_func(void (*draw_func)(int first_flag));
extern void lcd_default_draw_func(void);
extern void lcd_brightness(int brightness);

#endif /* !LCD_INC */
