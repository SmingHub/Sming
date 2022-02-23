AnimatedGIF
===========

.. highlight:: c++

Introduction
------------

AnimatedGIF is an optimized library for playing animated GIFs on embedded devices.

Features
--------

- Supports any MCU with at least 24K of RAM (Cortex-M0+ is the simplest I've tested).
- Optimized for speed; the main limitation will be how fast you can copy the pixels to the display. You can use SPI+DMA to help.
- GIF image data can come from memory (FLASH/RAM), SDCard or any media you provide.
- GIF files can be any length, (e.g. hundreds of megabytes)
- Simple C++ class and callback design allows you to easily add GIF support to any application.
- The C code doing the heavy lifting is completely portable and has no external dependencies.
- Does not use dynamic memory (malloc/free/new/delete), so it's easy to build it for a minimal bare metal system.


Using
----- 

1. Add ``COMPONENT_DEPENDS += AnimatedGIF`` to your application componenent.mk file.
2. Add these lines to your application::

	#include <AnimatedGifTask.h>
	
	namespace
	{
	AnimatedGifTask* task;
	
	// ...
	
	} // namespace
		
	void init()
	{
	
		initDisplay();
		tft.setOrientation(Graphics::Orientation::deg270);
	
		task = new AnimatedGifTask(tft.createSurface(), (uint8_t*)gifData.data(), gifData.length(), true);
		task->resume();
	}
