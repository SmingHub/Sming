I2S: Inter-IC Serial communcations
==================================

Introduction
------------

`I2S <https://en.wikipedia.org/wiki/I%C2%B2S>`__ was designed for transfer of digital audio data.

The ESP8266 has two I2S modules (one transmitter and one receiver), both with hardware
`DMA <https://en.wikipedia.org/wiki/Direct_memory_access>`__ support, which means transfers from
RAM to the hardware SPI FIFO can be handled directly in hardware without any CPU involvement.


Sming I2S support
-----------------

The Sming driver deals with the complicated of setting up the hardware, using an API
similar to that in the Espressif RTOS SDK. In addition, DMA buffers may be accessed directly
to avoid double-buffering and the associated RAM and copy overhead.


Applications
------------

Audio
~~~~~

Playing MIDI files, MP3 files, speech synthesis, etc. is all possible using the ESP8266,
though many audio applications require considerable processing power.
That means you may need to disable WiFi and set the processor to run at full 160MHz speed.

High-quality multi-channel audio requires an external I2S DAC, which is what the protocol
was designed for in the first place. You may find problems with insufficient RAM,
but you can always add external SPI RAM.

More realistic uses include generating simple tones, beeps, playing pre-recorded WAV audio,
etc. to supplement existing projects. This can all be done in the background without
disrupting the system's main purpose, whatever that may be.

For such applications you can generate single-channel audio via the I2S OUT pin,
using `Pulse-density modulation <https://en.wikipedia.org/wiki/Pulse-density_modulation>`__.

See the :library:`ToneGenerator` library for a demonstration of this.

   
GPIO Expansion
~~~~~~~~~~~~~~

Expand GPIO using low-cost shift registers. https://github.com/lhartmann/esp8266_reprap.


Pixel-strip control
~~~~~~~~~~~~~~~~~~~

Devices such as WS2812-based NeoPixels use a simple, single-wire protocol.
I2S is ideal for this as it can be used to generate a precisely-timed bitstream
with very low CPU loading.


API Documentation
-----------------

.. doxygengroup:: i2s_driver
   :content-only:
