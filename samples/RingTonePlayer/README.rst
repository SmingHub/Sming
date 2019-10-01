RingTone Player
===============

Introduction
------------

You may find the need to include basic audio features such as haptic feedback
or tune generation to support an existing application. The advantage of using I2S
for this is the very low CPU overhead.

This sample demonstrates how simple audio can be added to an application using the
I2S ToneGenerator and RingTone libraries. The sample contains a selection of tunes.

The sample may be controlled via serial terminal, or by web interface.

Serial connection
-----------------

Because I2S uses the serial RX pin, the serial port uses the alternate pin mappings.
See :library:`ToneGenerator` for connection details.

You'll still need the regular serial port for programming. Here's a sample setup for Windows:

- Using NodeMCU with standard serial port connected to COM4
- Second USB/UART converter connected to alternate serial pins as COM8

The command line to build, flash and open a terminal would be:

.. code-block:: cmd

   make -j COM_PORT=COM8 COM_SPEED=115200 COM_PORT_ESPTOOL=COM4 COM_SPEED_ESPTOOL=921600 SPI_MODE=dio SPI_SIZE=4M
   make flashinit
   make flash


Web interface
-------------

In addition to play controls, there are three graphs showing some statistics over a two-minute period.
The data is sent from the ESP8266 via websocket once per second.

CPU Usage
   Gives an indication of processor loading. Try connecting a second web client to see what happens.

Fill Time
   Tones are created as required and stored in Tone Buffers, which are then simply copied into
   the I2S DMA buffers during the interrupt service routine. That happens every 11ms and takes about 50us.
   (Note that this could be done using a task callback instead, but you'd generally need more RAM
   to cope with the higher latency.)

   This graph has three traces, indicating time in milliseconds over the 1-second update period:

   - Red: Longest time taken
   - Green: Shortest time
   - Black: Average time

   You'll see more activity during complex, fast tunes but also for lower notes which require more
   samples (and larger buffers).
   
   The progress bar indicates the total time taken over the update period.

Free Memory
   The graph has three traces:
   
   Black: Available memory
   Red: Maximum memory used during the update period
   Green: Memory in use at the time of update

   The red/green traces generally follow each other and represent the allocation/deallocation of
   tone buffers.

Notes
-----

Pausing playback stops the tune, but *Stop* also releases any allocated Tone Buffer memory and
stops the I2S transfers. However, I2S DMA buffers remain allocated.
