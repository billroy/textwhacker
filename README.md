# Textwhacker readme

Textwhacker enables Bitlash to print scrolling marquee text to a Sparkfun LED matrix.

## Install

Install Bitlash per instructions at http://bitlash.net

Install this textwhacker library in the same place you put Bitlash

Connect your Sparkfun LED matrix per these notes in textwhacker.cpp,
or change these lines to be appropriate for your hardware setup:

	//////////
	// Pin Configuration for software SPI interface
	//
	#define DATAOUT 	15	// MOSI
	#define SPICLOCK	14	// SCK
	#define SLAVESELECT 16	// ~SS, active low
	
	#define BOARD_VERSION	20	// 20 for 2.0, 25 for 2.5
	#define NUMBOARDS		1	// one board please

Restart the Arduino IDE (!) and open:
	File -> Examples -> textwhacker -> bitlash_textwhacker

Upload to your Arduino

Connect with a serial monitor and play with the new functions

##  New functions

#### text("Agent number: %03d", 7)

Formats the given arguments like printf() would and displays the resulting text in scrolling marquee format on the LED array.


#### fgcolor(color("red")) and bgcolor(color("blue"))

Set the foreground/background color for text.

Because it's inconvenient to memorize the color codes a translation function named color() is provided.  It knows the color map for the display.  Use it to look up the color number for you as in the examples above.

#### speed(50)

Sets the text scroll rate, more properly the pixel scroll interval, default is 50 ms.  

Smaller values make it go faster.

#### fill(color("white"))

Fills the display with the given color

#### color("red")

Returns the color code for the given color name.

These color names are supported:

	black
	blue
	cyan
	green
	magenta
	red
	white
	yellow




