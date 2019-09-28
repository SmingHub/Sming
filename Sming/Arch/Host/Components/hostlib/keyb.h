/**
 * keyb.h - Utility module for linux/Windows keyboard use
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * Usage
 * -----
 *
 * Call keyb_raw() to initialise, then call getkey() to fetch a keypress. Will return
 * an ASCII code or one of the KEY_xxx codes defined below.
 *
 * We expect keyboard input to be ANSI / TELNET compatible.
 *
 ****/

#pragma once

// Standard codes
#define KEY_NONE -1
#define KEY_BKSP 0x0008
#define KEY_TAB 0x0009
#define KEY_ESC 0x001b
#define KEY_DEL 0x007f
// 16-bit conversions for various escaped codes
// ESC [
#define KEY_F1 0x0131	// 1B 5B 31 31 7E
#define KEY_F2 0x0132	// 1B 5B 31 32 7E
#define KEY_F3 0x0133	// 1B 5B 31 33 7E
#define KEY_F4 0x0134	// 1B 5B 31 34 7E
#define KEY_F5 0x0135	// 1B 5B 31 35 7E
#define KEY_F6 0x0137	// 1B 5B 31 37 7E
#define KEY_F7 0x0138	// 1B 5B 31 38 7E
#define KEY_F8 0x0139	// 1B 5B 31 39 7E
#define KEY_F9 0x0230	// 1B 5B 32 30 7E
#define KEY_F10 0x0231   // 1B 5B 32 31 7E
#define KEY_F11 0x0233   // 1B 5B 32 33 7E
#define KEY_F12 0x0234   // 1B 5B 32 34 7E
#define KEY_HOME 0x0331  // 1B 5B 31 7E
#define KEY_INS 0x0332   // 1B 5B 32 7E
#define KEY_END 0x0334   // 1B 5B 34 7E
#define KEY_PGUP 0x0335  // 1B 5B 35 7E
#define KEY_PGDN 0x0336  // 1B 5B 36 7E
#define KEY_UP 0x0441	// 1B 5B 41
#define KEY_DOWN 0x0442  // 1B 5B 42
#define KEY_RIGHT 0x0443 // 1B 5B 43
#define KEY_LEFT 0x0444  // 1B 5B 44
// ESC O : GL encodes G3 for next character only
#define KEY_CTUP 0x0541	// 1B 4F 41
#define KEY_CTDOWN 0x0542  // 1B 4F 42
#define KEY_CTRIGHT 0x0543 // 1B 4F 43
#define KEY_CTLEFT 0x0544  // 1B 4F 44

void keyb_restore();
void keyb_raw();
int getch();
int getkey();

// For debugging
const char* getkeyname(int key);
void printkey(int key);
