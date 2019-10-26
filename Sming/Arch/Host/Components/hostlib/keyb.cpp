/**
 * keyb.cpp - Utility module for linux/Windows keyboard use
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
 ****/

#include "keyb.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef __WIN32

#define _NO_OLDNAMES
#include <conio.h>

#else

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static bool g_orig_values_saved, g_values_changed;
static struct termios g_orig_attr;
static int g_orig_flags;

#endif

/*
 * Simplifies interpretation of ANSI escape sequences for keyboard control.
 */
class CKeycode
{
public:
	CKeycode()
	{
		reset();
	}

	void reset()
	{
		m_esc = '\0';
		m_count = 0;
		m_buffer[0] = '\0';
	}

	// Return interpreted keycode or KEY_NONE
	int add(int c);

private:
	char m_esc;
	char m_buffer[32];
	unsigned m_count;

	void push(char c)
	{
		if(m_count < sizeof(m_buffer)) {
			m_buffer[m_count++] = c;
		}
	}
};

/*
 * Processes extended key sequences.
 *
 * Return KEY_NONE if no key pressed.
 */
int CKeycode::add(int c)
{
#ifdef __WIN32
	if(m_count != 0) {
		int cp = m_buffer[0];
		reset();
		if(cp == 0x00)
			switch(c) {
			case 0x3B:
				return KEY_F1;
			case 0x3C:
				return KEY_F2;
			case 0x3D:
				return KEY_F3;
			case 0x3E:
				return KEY_F4;
			case 0x3F:
				return KEY_F5;
			case 0x40:
				return KEY_F6;
			case 0x41:
				return KEY_F7;
			case 0x42:
				return KEY_F8;
			case 0x43:
				return KEY_F9;
			case 0x44:
				return KEY_F10;
			case 0x45:
				return KEY_F11;
			case 0x46:
				return KEY_F12;
			default:
				return KEY_NONE;
			}

		if(cp == 0xE0)
			switch(c) {
			case 0x47:
				return KEY_HOME;
			case 0x52:
				return KEY_INS;
			case 0x53:
				return KEY_DEL;
			case 0x4F:
				return KEY_END;
			case 0x49:
				return KEY_PGUP;
			case 0x51:
				return KEY_PGDN;
			case 0x48:
				return KEY_UP;
			case 0x50:
				return KEY_DOWN;
			case 0x4B:
				return KEY_LEFT;
			case 0x4D:
				return KEY_RIGHT;
			default:
				return KEY_NONE;
			}

		return KEY_NONE;
	}

	if(c == 0x00 || c == 0xE0) {
		push(c);
		return KEY_NONE;
	}

	return c;

#else

// Values for m_esc
#define ESC_NONE 0x00
#define ESC_START 0x01
#define ESC_SS2 'N'
#define ESC_SS3 'O'
#define ESC_DCS 'P'
#define ESC_CSI '['

	int ret = KEY_NONE;

	switch(m_esc) {
	case ESC_NONE:
		if(c == KEY_ESC)
			m_esc = ESC_START;
		else
			ret = c;
		break;

	case ESC_START:
		if(c == KEY_NONE || c == KEY_ESC) {
			ret = KEY_ESC;
			m_esc = ESC_NONE;
		} else
			m_esc = c;
		break;

	/*
    The ESC [ is followed by any number (including none) of "parameter bytes" in the range 0x30–0x3F (ASCII 0–9:;<=>?),
    then by any number of "intermediate bytes" in the range 0x20–0x2F (ASCII space and !"#$%&'()*+,-./), then finally
    by a single "final byte" in the range 0x40–0x7E (ASCII @A–Z[\]^_`a–z{|}~).
  */
	case ESC_CSI:
		if(c < 0x40) {
			push(c);
			break;
		}
		//#define KEY_UP      0x0441           // 1B 5B 41
		//#define KEY_HOME    0x0331           // 1B 5B 31 7E
		//#define KEY_F1      0x0131           // 1B 5B 31 31 7E

		if(m_count == 0)
			ret = 0x0400 | c;
		else if(m_count == 1)
			ret = 0x0300 | m_buffer[0];
		else if(m_count == 2)
			ret = ((m_buffer[0] & 0x0f) << 8) | m_buffer[1];
		else
			; //!! Unexpected

		reset();
		break;

	case 0x4f:
		// Ctrl+ cursor keys
		ret = 0x0500 | c;
		break;

	default:
		// Unexpected
		reset();
		//    if (c != KEY_NONE) {
		//        // Normal character following ESCape key
		//        ungetc(ext, stdin);
		//      }
		//    }
	}

	return ret;

#endif
}

void keyb_restore()
{
#ifndef __WIN32
	if(g_values_changed) {
		static struct termios attr = g_orig_attr;
		attr.c_lflag |= ICANON | ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &attr);

		(void)fcntl(0, F_SETFL, g_orig_flags);
		g_values_changed = false;
	}
#endif
}

void keyb_raw()
{
#ifndef __WIN32
	if(!g_orig_values_saved) {
		// make stdin non-blocking
		g_orig_flags = fcntl(0, F_GETFL);
		tcgetattr(STDIN_FILENO, &g_orig_attr);
		atexit(keyb_restore);
	}

	// make stdin non-blocking
	(void)fcntl(0, F_SETFL, g_orig_flags | O_NONBLOCK);
	//
	struct termios attr = g_orig_attr;
	attr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &attr);

	g_values_changed = true;
#endif
}

int getch()
{
#ifdef __WIN32
	return _kbhit() ? _getch() : -1;
#else
	return getchar();
#endif
}

int getkey()
{
	static CKeycode kc;
	int c;
	for(;;) {
		c = getch();
		if(c == KEY_NONE)
			break;
		c = kc.add(c);
		if(c != KEY_NONE)
			break;
	}
	return c;
}

const char* getkeyname(int key)
{
	static char buffer[32];

#define KEY_TEST(key)                                                                                                  \
	case KEY_##key:                                                                                                    \
		return #key;

	switch(key) {
		KEY_TEST(ESC)
		KEY_TEST(F1);
		KEY_TEST(F2);
		KEY_TEST(F3);
		KEY_TEST(F4);
		KEY_TEST(F5);
		KEY_TEST(F6);
		KEY_TEST(F7);
		KEY_TEST(F8);
		KEY_TEST(F9);
		KEY_TEST(F10);
		KEY_TEST(F11);
		KEY_TEST(F12)
		KEY_TEST(HOME)
		KEY_TEST(INS)
		KEY_TEST(DEL)
		KEY_TEST(END)
		KEY_TEST(BKSP)
		KEY_TEST(PGUP)
		KEY_TEST(PGDN)
		KEY_TEST(UP)
		KEY_TEST(DOWN)
		KEY_TEST(RIGHT)
		KEY_TEST(LEFT)
		KEY_TEST(CTUP)
		KEY_TEST(CTDOWN)
		KEY_TEST(CTRIGHT)
		KEY_TEST(CTLEFT)

	default:
		sprintf(buffer, "0x%04X (%c)", key, isprint(key) ? key : '?');
		return buffer;
	}
}

void printkey(int key)
{
	puts(getkeyname(key));
}
