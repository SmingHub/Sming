# ILI9163C TFT Display

ILI9163C- A fast SPI driver for TFT that use Ilitek ILI9163C.

![ILI9163C](http://i1189.photobucket.com/albums/z437/theamra/github/CIMG6810.jpg)

	Link to a video:
	
https://www.youtube.com/watch?v=y5f-VNBxgEk&feature=youtu.be
	
==========================

Features:
	
	- Very FAST!, expecially with Teensy 3.x where uses DMA SPI.
	- It uses just 4 or 5 wires.
	- Compatible at command level with Adafruit display series so it's easy to adapt existing code.
	- It uses the standard Adafruit_GFX Library (you need to install). 
	

Pay Attention to connections!!!!:
	
	- This display has logic at 3V3 volt so YOU NEED A VOLTAGE CONVERTER if you plan to use with arduino.
	If you try to connect directly you can burn it very fast so PAY ATTENTION!
	- My display works at 3V3 volt for logic but LED background has resistor for 5V. Your can be different
	so carefully check out before connect it.
	- My library works only in SPI mode by using MOSI,SCLK and a CS pin plus an additional pin for DC (or RS).
	I've used also the reset pin but you can save it by connect it at 3V3 volt and use the constructor without
	the reset pin. The initialization routine will automatically use the software reset.
	- People using Teensy3 should remember that have to choose for CS and DC a pin that satisfy:

	if (pin == 2 || pin == 6 || pin == 9) return true;
	if (pin == 10 || pin == 15) return true;
	if (pin >= 20 && pin <= 23) return true;


Background:
	
	I got one of those displays from a chinese ebay seller but unfortunatly I cannot get
	any working library so I decided to hack it. ILI9163C looks pretty similar to other 
	display driver but it uses it's own commands so it's tricky to work with it unlsess you
	carefully fight with his gigantic and not so clever datasheet.
	My display it's a 1.44"", 128x128 that suppose to substitute Nokia 5110 LCD and here's the 
	first confusion! Many sellers claim that it's compatible with Nokia 5110 (that use a philips
	controller) but the only similarity it's the pin names since that this one it's color and
	have totally different controller that's not compatible. Altrough I discovered that it's not
	128x128 but 128x160 (!??)... Check links below to see if it's similar to yours.
	
http://www.ebay.com/itm/Replace-Nokia-5110-LCD-1-44-Red-Serial-128X128-SPI-Color-TFT-LCD-Display-Module-/141196897388

http://www.elecrow.com/144-128x-128-tft-lcd-with-spi-interface-p-855.html
	
	Pay attention that ILI9163C can drive different resolutions and your display can be
	160*128 or whatever, also there's a strain of this display with a black PCB that a friend of mine
	got some weeks ago and need some small changes in library to get working.
	If you look at TFT_ILI9163C.h file you can add your modifications and let me know so I
	can include for future versions.
	
Code Optimizations:
	
	The purpose of this library it's SPEED. I have tried to use hardware optimized calls
	where was possible and results are quite good for most applications.
	Of course it can be improved so feel free to add suggestions.
-------------------------------------------------------------------------------
    Copyright (c) 2014, .S.U.M.O.T.O.Y., coded by Max MC Costa.    

    TFT_ILI9163C Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TFT_ILI9163C Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

This file needs the following Libraries:
 
    *Adafruit_GFX by Adafruit:
    
https://github.com/adafruit/Adafruit-GFX-Library

    Remember to update GFX library often to have more features with this library!
	
    *From this version I'm using my version of Adafruit_GFX library:
    
https://github.com/sumotoy/Adafruit-GFX-Library

    It has faster char rendering and some small little optimizations but you can
    choose one of the two freely since are both fully compatible.
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Special Thanks:
	
	Thanks Adafruit for his Adafruit_GFX!
	Thanks to Paul Stoffregen for his beautiful Teensy3 and DMA SPI.
	
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Version:
	
	0.1a1: First release, compile correctly. Altrough not fully working!
	0.1a3: Some bugfix, still some addressing problems, partial rotation solved.
	0.1b1: Beta version! Fully working but still not tested with Arduino and DUE (altrough it compile)
	0.2b2: Code cleaned and added support for 2.2" RED PCB displays.
	0.2b4: Bug fixes and added color space support.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BugList of the current version:
	
	- Beta version fully working!
