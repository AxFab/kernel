/*
 *      This file is part of the SmokeOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Header file for Keyboard keys definitions.
 */
#pragma once

#define KEY_BACKSPACE 0x08
#define KEY_TAB 0x09
#define KEY_ENTER 0x0a
#define KEY_ESCAPE 0x0e
#define KEY_SH_RG 0x0f
#define KEY_SH_LF 0x10
#define KEY_CTRL 0x11
#define KEY_ALT 0x12

#define KEY_F1 0xdc
#define KEY_F2 0xdd
#define KEY_F3 0xde
#define KEY_F4 0xdf
#define KEY_F5 0xe0
#define KEY_F6 0xe1
#define KEY_F7 0xe2
#define KEY_F8 0xe3
#define KEY_F9 0xe4
#define KEY_F10 0xe5
#define KEY_F11 0xe6
#define KEY_F12 0xe7

#define KEY_SCROLL 0xe8
#define KEY_INSERT 0xe9
#define KEY_DELETE 0xea
#define KEY_NUMLOCK 0xeb

#define KEY_PAUSE 0xec

#define KEY_CAPSLOCK 0xf0

#define KEY_PAGE_UP 0xf2
#define KEY_PAGE_DW 0xf3
#define KEY_ARROW_UP 0xf4
#define KEY_ARROW_RG 0xf5
#define KEY_ARROW_DW 0xf6
#define KEY_ARROW_LF 0xf7
#define KEY_HOME 0xf8
#define KEY_END 0xf9

#define KEY_HOST 0xfa
#define KEY_MENU 0xfb


/** Table for conversion from 'IBM AT-Style Keyboard' to system keys */
extern unsigned char key_layout_us[128][4];

