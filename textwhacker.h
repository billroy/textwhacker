/***
	textwhacker.h - Text display driver library for SparkFun 8x8 RGB Serial/SPI LED Array

	Copyright (C) 2009-2011 Palmeta Software LLC

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

	The author can be contacted at bill@bitlash.net.

***/
#include "WProgram.h"
#include "avr/pgmspace.h"


// External function interface definitions
//
void initTextwhacker(void);
void runTextwhacker(void);

void initDisplayBuffer(void);
void displayPutChar(byte);

void showText(char *text);
byte waitDisplayIdle(void);
void fill(byte);
byte getcolor(char *);

extern char displaytext[];
extern unsigned long scrolldelay;
extern byte fgcolor, bgcolor;


