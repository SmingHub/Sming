Software timer queues
=====================

Sming has one timer queue which is managed via the :cpp:class:`Timer` and :cpp:type:`SimpleTimer` classes.

Like hardware timers, these are callback timers, but the callback handlers execute in the
*task context* so there are no special restrictions on what you can do there.

Because of this, however, these timers are less precise hardware timers, and the accuracy
generally depends on how busy the CPU is.

Timers can be 'one-shot', for timing single events, or 'auto-reset' repetitive timers.
A repetitive timer will ensure that time intervals between successive callbacks are consistent.


.. toctree::

   timer
   simple-timer
