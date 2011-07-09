/***
	textwhacker.cpp - Text display driver library for SparkFun 8x8 RGB Serial/SPI LED Array

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
#include "textwhacker.h"

//////////
// Pin Configuration for software SPI interface
//
#define DATAOUT 	15	// MOSI
#define SPICLOCK	14	// SCK
#define SLAVESELECT 16	// ~SS, active low

#define BOARD_VERSION	20	// 20 for 2.0, 25 for 2.5
#define NUMBOARDS		1	// one board please



///////////////////////////////////////
//	Fonts
//
// fonts from LED Sign freeware:
// http://www.cs.dartmouth.edu/~wbc/led/COPYRIGHT
// processed to .h files using fontmkr.py, q.v.
//
//#include "tinyfont.h"
#include "8font.h"



////////////////////////////////////////////////////////////////
//
//	Interface Code for Sparkfun Serial LCD Matrix v2.0 and v2.5
//

// Backpack color map for 2.0
#define BLACK_20	0
#define RED_20		1
#define GREEN_20	2
#define BLUE_20		3
#define YELLOW_20	4
#define CYAN_20		5
#define MAGENTA_20	6
#define WHITE_20	7

// Backpack color map for 2.5
#define RED_SHIFT	5
#define MAX_RED		7
#define RED (MAX_RED<<RED_SHIFT)
#define GREEN_SHIFT	2
#define	MAX_GREEN	7
#define GREEN (MAX_GREEN<<GREEN_SHIFT)
#define BLUE_SHIFT	0
#define MAX_BLUE	3
#define BLUE (MAX_BLUE<<BLUE_SHIFT)

#define YELLOW	(RED | GREEN)
#define CYAN	(BLUE | GREEN)
#define MAGENTA	(RED | BLUE)
//#define WHITE (RED | BLUE | GREEN)			// this looks very red on the units here
#define WHITE ((1<<RED_SHIFT) | BLUE | GREEN)	// still pink but better
#define BLACK	0

#define NUMCOLORS 8
prog_char colorcodes_20[NUMCOLORS] PROGMEM = { BLACK_20, BLUE_20, CYAN_20, GREEN_20, MAGENTA_20, RED_20, WHITE_20, YELLOW_20 };
prog_char colorcodes_25[NUMCOLORS] PROGMEM = { BLACK, BLUE, CYAN, GREEN, MAGENTA, RED, WHITE, YELLOW };
prog_char colornames[] PROGMEM = { "black\0blue\0cyan\0green\0magenta\0red\0white\0yellow\0" };

#define DEFAULT_COLOR_VERSION colorcodes_25
prog_char *colortab = DEFAULT_COLOR_VERSION;

// Color lookups for use in color references
#define COLOR_BLACK 	pgm_read_byte(colortab + 0)
#define COLOR_BLUE 		pgm_read_byte(colortab + 1)
#define COLOR_CYAN 		pgm_read_byte(colortab + 2)
#define COLOR_GREEN 	pgm_read_byte(colortab + 3)
#define COLOR_MAGENTA 	pgm_read_byte(colortab + 4)
#define COLOR_RED 		pgm_read_byte(colortab + 5)
#define COLOR_WHITE 	pgm_read_byte(colortab + 6)
#define COLOR_YELLOW 	pgm_read_byte(colortab + 7)

// Default colors
byte fgcolor, bgcolor;


/////////////////////////////
// Compatibility for Sparkfun Serial Matrix v2.0 and 2.5

void setVersion(byte version) {
	if (version == 20) colortab = colorcodes_20;
	else {
		colortab = colorcodes_25;
		version = 25;
	}
}

// find id in PROGMEM wordlist.  result in symval, return bool found.
unsigned char colorval;
char findcolor(char *id, prog_char *wordlist, unsigned char sorted) {
	colorval = 0;
	while (pgm_read_byte(wordlist)) {
		int result = strcmp_P(id, wordlist);
		if (!result) return 1;
		else if (sorted && (result < 0)) break;	// only works if list is sorted!
		else {
			colorval++;
			wordlist += strlen_P(wordlist) + 1;
		}
	}
	return 0;
}


//////////////////////////////////
// Parse a color specifier: a number, or a name from the color table
//
byte getcolor(char *arg) {
	if ((*arg >= '0') && (*arg <= '9')) return atol(arg);
	if (!findcolor(arg, colornames, 1)) colorval = 5;	// not found? say RED
	return pgm_read_byte(colortab + colorval);
}



///////////////////////////////
// SPI interface
// Bitbang software SPI
//
void spi_start(void) {
	digitalWrite(SLAVESELECT, LOW);
	delay(1);
}

void spi_end(void) {
	delay(1);
	digitalWrite(SLAVESELECT, HIGH);
}

void spi_init(void) {
	pinMode(DATAOUT, OUTPUT);		digitalWrite(DATAOUT, LOW);
	pinMode(SPICLOCK, OUTPUT);		digitalWrite(SPICLOCK, LOW);
	pinMode(SLAVESELECT, OUTPUT);	digitalWrite(SLAVESELECT, HIGH);
}

// spi clock delay time PER HALF CYCLE
// per data sheet the clock must be less than 125KHz
// or 8 microseconds per cycle minimum
// this constant is the half cycle
#define SPI_CLK_DELAY_US 4

void spi_put(char b) {
	int i;
	for (i=0; i<8; i++) {
		if (b & 0x80) digitalWrite(DATAOUT, HIGH);
		else digitalWrite(DATAOUT, LOW);

		// toggle the SPI clock, respecting the 125KHz limit
		digitalWrite(SPICLOCK, HIGH);
		delayMicroseconds(SPI_CLK_DELAY_US);
		digitalWrite(SPICLOCK, LOW);	
		delayMicroseconds(SPI_CLK_DELAY_US);

		b <<= 1;
	}
}


///////////////////////////////////////////
//	Set the number of boards (version 2.5)
//
//	Updating the backpack's NUMBOARDS with the % command
//	is done in doCommand so it only happens once, when the
//	command is issued (and not, therefore, at each startup).
//
uint8_t numboards;

void setNumBoards(uint8_t new_numboards) {
	if ((new_numboards == 0) || (new_numboards > 8)) numboards = 1;
	else numboards = new_numboards;
}

// delay between board frames in a multi board setup (ms)
#define INTER_BOARD_DELAY 10

void fill(byte color) {
int b, i;
	for (b=0; b < numboards; b++) {
		if (b) delay(INTER_BOARD_DELAY);
		spi_start();
		for (i = 0; i < 64; i++) spi_put(color);
		spi_end();
	}
}



///////////////////////////////////
//
// Display state machine
//
// Your state function looks just like void loop(void) {...}
typedef void (*statefn)(void);

statefn currentstate;
unsigned long startmillis;
unsigned long endsnooze;

void set_state(statefn);
void snooze(int);
void idle(void);
void dwell(void);


// How many millis have we been in this state?
unsigned long stateMillis(void) { return millis() - startmillis; }

// Set the current state, i.e., register a new procedure to be the "current state handler"
// Thus ping() cedes to pong() and vice versa
void set_state(statefn func) {
  currentstate = func;
  startmillis = millis();
}

// Snooze: Suspend this state machine for (int) millis
void snooze(int ms) { 
	endsnooze = millis() + ms; 
}

void init_state(statefn initialState) {
	set_state(initialState);
	snooze(0);		// mark runnable
}

byte displayBusy(void) {
	return (currentstate != &idle);
}

byte waitDisplayIdle(void) {
	while (displayBusy()) runTextwhacker();
}



// Font API

int getCharWidth(char c) {
	int index = c - 32;
	if ((index < 0) || (index > (127-32))) index = 0;
	int w = pgm_read_byte(fontwidths + index);
	return w + 1;
}

int getCharData(char c, int row, int offset) {
	int index = c - 32;
	if ((index < 0) || (index > (127-32))) index = 0;
	
	// The Sparkfun version 2.5 display has the row order numbering reversed, compared with
	// the earlier v2.0 display.  We suck that difference up here.
	if (colortab == colorcodes_25) row = (7 - row) & 7;

	int b = pgm_read_byte(fontbits + (index * 8) + row);
	return b << offset;
}


// Scrolling text marquee

#define DISPLAYBUFLEN 80
char displaytext[DISPLAYBUFLEN];
char *t;
int ho;		// the mighty horizontal offset [0..w]
#define SCROLLDELAY 50
unsigned long scrolldelay = SCROLLDELAY;

void scrolling(void) {
	snooze(scrolldelay);
	uint8_t board_offset;
	for (board_offset = 0; board_offset < (8*numboards); board_offset = board_offset + 8) {
		if (board_offset) delay(10);			// interframe delay, per data sheet
		spi_start();
		uint8_t r;
		for (r=0; r < 8; r++) {
			//produce all the columns of one row
			int o = ho;						// start with the global offset
			char *txt = t;	
			uint8_t w = getCharWidth(*txt);
			uint8_t b = getCharData(*txt, r, ho);
			uint8_t c = 0; 
			while (c < (board_offset + 8)) {
				if (o <= w) {		 // any bits left?
					if (c >= board_offset) {
						spi_put((b & 0x80) ? fgcolor : bgcolor);
					}
					c++;			// next column
					o++;			// ... using next bit of this one
					b = (b << 1);		// ...teed up right here
				} else {			 // advance to next char
					if (*txt) ++txt;
					w = getCharWidth(*txt);
					b = getCharData(*txt, r, 0);
					o = 0;
				}
			}
		}
		spi_end();
	}

	// update offset and char for next frame
	if (++ho > getCharWidth(*t)) {
		ho = 0;
		if (*t) ++t;		// on to the next char
	}

	if (!(*t)) {
		snooze(0);	// cancel the iso-snooze above
		set_state(dwell);
	}
}


//////////
//
// showText: Animate text in displaytext as scrolling text
//
void showText(char *text) {
	waitDisplayIdle();
	if (text != displaytext)
		strncpy(displaytext, text, DISPLAYBUFLEN);
	t = displaytext;
	ho = 0;
	set_state(scrolling);
}


// Character interface for display buffer
//
char *dptr;

void initDisplayBuffer(void) {
	dptr = displaytext;
	*dptr = 0;
}

void displayPutChar(byte c) {
	if (dptr < &displaytext[DISPLAYBUFLEN-1]) {
		if (c >= ' ') *dptr++ = c;
		*dptr = 0;
	}
}

void idle(void) {
	snooze(1000000L);	// chill out until next call to showText
}

#define DWELLWAIT 1000UL

//////////
//
// dwell: insert a short blank interval after the scrolling completes
//
void dwell(void) {
	if (stateMillis() >= DWELLWAIT) set_state(idle);
	else snooze(50);
}




//////////////////
//
// Initialization
//
void initTextwhacker(void) {
	fgcolor = COLOR_WHITE;
	bgcolor = COLOR_BLACK;
	spi_init();
	setVersion(BOARD_VERSION);
	setNumBoards(NUMBOARDS);	// default to one board
	init_state(idle);
	snooze(1000);		// give the blinky thing time to start up
	delay(3000);		// the smiley face startup is cute, but she hurts us here
}



// This must be called in loop(), frequently, to run the state machine
void runTextwhacker(void) {
	if (millis() >= endsnooze) {
		if (currentstate) {
			(*currentstate)();
		}
	}
}

