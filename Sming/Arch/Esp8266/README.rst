Sming Esp8266 Architecture
==========================

Support building Sming for the Esp8266 architecture.

This is the SOC for which Sming was originally developed,
so many of the low-level API calls reflect those in the
`SDK </_inc/Sming/Arch/Esp8266/Components/esp8266/README>`.
These include functions such as
:c:func:`system_get_free_heap_size`,
:c:func:`system_update_cpu_freq`,
:c:func:`system_get_cpu_freq`,
:c:func:`esp_get_ccount`
and others.

The Esp8266 is the device which popularised 32-bit microcontrollers
with integrated WiFi capability, the so-called
`System on a chip <https://en.wikipedia.org/wiki/System_on_a_chip>`__ (SoC).


Features
--------

-  Integrated boot loader :component:`rboot` with support for 1MB ROMs, OTA firmware updating and ROM switching.
-  :doc:`Crash handlers <debugging/index>` for analyzing/handling system restarts due to fatal errors or WDT resets.
-  :component-esp8266:`PWM support <driver>` based on `Stefan Bruens PWM <https://github.com/StefanBruens/ESP8266_new_pwm.git>`__.
-  Optional :component-esp8266:`custom heap allocation <heap>` based on `Umm Malloc <https://github.com/rhempel/umm_malloc.git>`__.
-  Based on :component-esp8266:`Espressif NONOS SDK Version 3 <esp8266>`.


Characteristics
---------------

Compared with more recent offerings such as the
:doc:`ESP32 </_inc/Sming/Arch/Esp32/README>` and
:doc:`RP2040 </_inc/Sming/Arch/Rp2040/README>`,
it has characteristics which require special consideration:

Limited RAM
   Typically around 50 KiB of RAM is available for user applications,
   compared with over 200 KiB for the Rp2040 and 300 KiB for the Esp32.
   Although applications not requiring WiFi can recover another 30 KiB
   via :component:`esp_no_wifi`, careful use of RAM is a primary consideration.
   See :doc:`/information/memory`.
Word-aligned SPI flash accesses
   Flash memory is relatively plentiful, provided via high-speed serial SPI.
   Because it is not internal to the SoC, the Esp8266 (and others) have hardware which
   supports mapping areas of external memory into the address space so that
   code and data can be accessed using standard machine instructions.
   The Esp8266 has a particular quirk in that all such accesses must be
   properly aligned to 32-bit word boundaries; failure to do this crashes the
   system.
   Other microcontrollers have better caching which handles misaligned accesses
   without issue. See :doc:`/information/flash'.
Weak hardware peripheral support
   Even compared to older Atmel or Microchip 8-bit microcontrollers,
   hardware support for interfaces such as I2C and PWM is lacking and these must
   be implemented in software.


Configuration Variables
-----------------------

.. envvar:: ESP_HOME

   This contains the base directory for the toolchain used to build the framework.
   See also :doc:`/arch/esp8266/getting-started/eqt`.


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
