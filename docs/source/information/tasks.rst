Tasks
=====

Introduction
------------

If you need to perform any kind of background processing *task*, then you will need
to consider how it to break it up so it executes in small slices to allow other
system functions to continue normally.

Sming has a *task queue* which is integrated with the ESP8266 SDK to ensure
system tasks (such as WiFi) get handled properly. Applications can use it by
calling `System.queueCallback()`:

.. doxygenclass:: SystemClass
   :members: queueCallback

Callbacks are executed as soon as possible, but the queue size is limited.
If necessary, it can be adjusted using TASK_QUEUE_LENGTH...

