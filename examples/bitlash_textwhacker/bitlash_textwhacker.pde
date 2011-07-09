//////////////////////////////////////////////////////////////////
//
//	bitlash_textwhacker.pde: Bitlash-Textwhacker integration
//
//	Copyright 2011 by Bill Roy
//
//	Permission is hereby granted, free of charge, to any person
//	obtaining a copy of this software and associated documentation
//	files (the "Software"), to deal in the Software without
//	restriction, including without limitation the rights to use,
//	copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following
//	conditions:
//	
//	The above copyright notice and this permission notice shall be
//	included in all copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//	OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////
//
#include "WProgram.h"
#include "bitlash.h"
#include "../../libraries/bitlash/src/bitlash.h"
#include "textwhacker.h"


numvar func_tprintf(void) {
	initDisplayBuffer();
	setOutputHandler(&displayPutChar);
	func_printf_handler(1,2);	// format=arg(1), optional args start at 2
	resetOutputHandler();
	showText(displaytext);
	return 0;
}

numvar func_scroll(void) {
	scrolldelay = getarg(1);
	return 0;
}

numvar func_fill(void) {
	fill(getarg(1));
	return 0;
}

numvar func_fgcolor(void) {
	fgcolor = getarg(1);
	return 0;
}

numvar func_bgcolor(void) {
	bgcolor = getarg(1);
	return 0;
}

numvar func_color(void) {
	return getcolor((char *) getarg(1));
}

void setup(void) {
	initBitlash(57600);		// must be first to initialize serial port
	initTextwhacker();

	// Scrolling text marquee
	addBitlashFunction("tprintf", 	(bitlash_function) func_tprintf);
	addBitlashFunction("fgcolor", 	(bitlash_function) func_fgcolor);
	addBitlashFunction("bgcolor", 	(bitlash_function) func_bgcolor);
	addBitlashFunction("scroll", 	(bitlash_function) func_scroll);

	// Fill display with a color
	addBitlashFunction("fill", 		(bitlash_function) func_fill);

	// Return color number for string color name, e.g.: fill(color("red"))
	addBitlashFunction("color", 	(bitlash_function) func_color);

	fill(getcolor("blue"));
	showText("Textwhacker here!");
}

void loop(void) {
	runTextwhacker();
	runBitlash();
}
