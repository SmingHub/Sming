Host Digital I/O
================

By default, the emulator just writes output to the console so you can see when outputs are changed.

Reading from an input returns 0.

All digital functions can be customised by overriding the *DigitalHooks* class, like this:

.. code-block:: c++

   // You'd probably put this in a separate module and conditionally include it
   #ifdef ARCH_HOST
   class MyDigitalHooks: public DigitalHooks
   {
   public:
      // Override class methods as required
      uint8_t digitalRead(uint16_t pin, uint8_t mode) override
      {
         if(pin == 0) {
            return 255;
         } else {
            return DigitalHooks::digitalRead(pin, mode);
         }
      }
   };
   
   MyDigitalHooks myDigitalHooks;
   #endif

   void init()
   {
      #ifdef ARCH_HOST
      setDigitalHooks(&myDigitalHooks);
      #endif
   }

See :source:`Sming/Arch/Host/Core/DigitalHooks.h` for further details.
