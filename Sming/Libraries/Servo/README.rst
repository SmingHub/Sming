Servo RC PWM Control
====================
 
Library to control `RC servos <https://en.wikipedia.org/wiki/Servo_(radio_control)>`__ with PWM signals.

Brief introduction
------------------

There are generally two types of servo actuator (digital and analogue) and this library supports
only the analogue variety.

Servo actuators have a logic-level control signal to set the position using an active-high pulse
of around 1 - 2 ms. This pulse must be repeated every 20ms or so to ensure the position is maintained

Servos are generally insensitive to the exact period provided it's no more than about 25ms.
It's the duration of the pulse which is critical.

For most servos 1.5ms is the default/centre position, however the min/max values will vary between
models depending on the exact type and range of motion. These values are therefore configurable for
each channel.


Physical connection
-------------------

Servos typically use a 5V logic-level input but are usually fine with the 3.3v output from the ESP8266.

.. warning::

   Like relays a servo is an electro-mechanical device, but it also has integrated control circuitry
   so doesn't require flyback diodes, etc. to protect from current spikes.

   However, remember to always insert a protection resistor of at least 200 ohms between the GPIO and
   the servo. This limits the current to <10mA if 5V is present on the line.
   For 12V servos a 1K resistor will peform the same function.


Technical Explanation
---------------------

Each servo actuator is connected to a GPIO pin, which is toggled by an ISR driven from the Hardware Timer.
The *ServoChannel* class represents this connection, and defines the current value (in microseconds)
plus the range (minimum and maximum values) to which the value is constrained.

The hardware timer interrupt is managed by a single instance of the *Servo* class.
All channels are updated sequentially at the start of each `frame` period (20ms in duration):

.. wavedrom::

   { "signal": [
           { "name": "#1", "wave": "h.l.........|" },
           { "name": "#2", "wave": "l.pl........|" },
           { "name": "#3", "wave": "l.nh...l....|" },
           { "name": "#4", "wave": "l......h.l..|" },
           { "name": "#5", "wave": "l........hl.|" }
   ]}


The first channel is set ON, then after the required time it is set OFF and the next channel set ON.
The final interrupt turns the active channel OFF. This requires (NumChannels + 1) interrupts per frame,
and the process repeats continuously whilst there is at least one active channel.

Channel updates (via calls to *ServoChannel::setValue()*) are not handled immeidately, but defered
using a 10ms software timer (half the frame period). This allows more efficient updating especially
where multiple channels are changed together.

A double-buffering technique is used for updates to avoid disabling interrupts, which allows use of
the non-maskable timer interrupts for best timing accuracy and eliminates glitches in the output.

Updates involve re-calculating the list of active pins and timer intervals, which is stored into
a second frame buffer. This is made active by the ISR when the current frame has completed.

If the ISR hasn't yet processed a previous update, it will be retried after a further 10ms.

