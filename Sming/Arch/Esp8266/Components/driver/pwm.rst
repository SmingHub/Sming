PWM: Pulse-Width Modulation
===========================

The driver interface is defined in the ESP8266 SDK.

Build variables
---------------

.. envvar:: ENABLE_CUSTOM_PWM

   undefined
      use the Espressif PWM driver

   1 (default)
      Use the *New PWM* driver, a drop-in replacement for the version provided in the Espressif SDK.

API Documentation
-----------------

.. doxygengroup:: pwm_driver
   :content-only:
