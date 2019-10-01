Tone Generator
==============

Uses the I2S module to synthesize tones using a minimal amount of Flash memory, RAM and CPU time.

Audio is output via the I2S_TX_DATA pin using `Delta-Sigma modulation <https://en.wikipedia.org/wiki/Delta-sigma_modulation>`__.
This is a high-frequency bitstream which requires low-pass filtering and suitable amplification for driving speaker/headphones.

.. warning::

   Do not connect the output directly to a loudspeaker or headphone as you may damage
   the GPIO. Always buffer the signal using a transistor, op-amp, etc.

See :component-esp8266:`driver` for further information about I2S.

As the output pin is normally used as UART0 RX, the alternate UART pin configuration is used:

====     ===========    =======     ===================
GPIO     Alternate      NodeMCU     Notes
====     ===========    =======     ===================
3        I2S_TX_DATA    D9          Audio output
13       RXD2           D7          Debug serial input
15       TXD2           D8          Debug serial output
====     ===========    =======     ===================

GPIO3 is still required for programming so you'll need to make sure the audio buffer is
disconnected, or has a reasonably high impedance load. You'll also need an additional
USB-serial converter connected to GPIO13 & 15 for terminal operation.
