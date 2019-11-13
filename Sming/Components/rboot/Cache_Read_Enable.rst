ESP8266 Cache_Read_Enable
=========================

12th June 2015 Richard

Since I haven’t seen this documented anywhere, here is my attempt to explain the Cache_Read_Enable function.
Valid values and what they do (at a register level) are from decompiling the code.
The outcome of those values is based on my own experimentation so my descriptions and explanations
may be silly but they currently fit the observed results.

void Cache_Read_Enable(uint8 odd_even, uint8 mb_count, unt8 no_idea);

Valid values for odd_even
   0 – clears bits 24 & 25 of control register 0x3FF0000C
   1 – clears bit 24, sets bit 25
   other – clears bit 25, sets bit 24

Function of odd_even
   0 – allows access to even numbered mb
   1 – allow access to odd numbered mb
   other – appears to do the same as 1, there must be a difference but I haven’t worked out what it is

Valid values for mb_count
   0-7 – set bits 16, 17 & 18 of control register 0x3FF0000C

Function of mb_count
   Which odd or even bank to map (according to odd_even option)
      e.g. mb_count = 0, odd_even = 0 -> map first 8Mbit of flash
      e.g. mb_count = 0, odd_even = 1 -> map second 8Mbit of flash
      e.g. mb_count = 1, odd_even = 0 -> map third 8Mbit of flash
      e.g. mb_count = 1, odd_even = 1 -> map fourth 8Mbit of flash

Valid values for no_idea
   0 – sets bit 3 of 0x3FF00024
   1 – sets bit 26 of 0x3FF0000C and sets bits 3 & 4 of 0x3FF00024

Function of no_idea
   The clue is in the name, I can’t work out what this does from my experiments, but the SDK always sets this to 1.


**Source: https://richard.burtons.org/2015/06/12/esp8266-cache_read_enable/**


13/11/2019 @author mikee47 UPDATE
---------------------------------

.. highlight:: c++

Ref. RTOS SDK source ``bootloader_support/bootloader_utility.c``,
function ``bootloader_utility_load_image()``::

   extern void Cache_Read_Enable(uint8_t map, uint8_t p, uint8_t v);
   ...
   Cache_Read_Enable(map, 0, SOC_CACHE_SIZE);
   
Where SOC_CACHE_SIZE is defined as::

   #ifdef CONFIG_SOC_FULL_ICACHE
   #define SOC_CACHE_SIZE 1 // 32KB
   #else
   #define SOC_CACHE_SIZE 0 // 16KB
   #endif


