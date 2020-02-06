Command Executor
================

.. highlight:: c++

Introduction
------------

Command handler provides a common command line interface. CLI is available for the following remote access methods:

- Serial
- Telnet
- Websockets

By default, CLI is disabled. Enable CLI by calling "commandProcessing" on the appropriate access class object, e.g::

   Serial.commandProcessing(true)

Commands can be added to and removed from the command handler. Each command will trigger a defined Delegate.

A welcome message may be shown when a user connects and end of line character may be defined. An automatic "help" display is available.

Build Variables
---------------

.. envvar:: ENABLE_CMD_EXECUTOR

   Default: 1 (ON)

   This feature enables execution of certain commands by registering token handlers for text
   received via serial, websocket or telnet connection. If this feature
   is not used additional RAM/Flash can be obtained by setting
   ``ENABLE_CMD_EXECUTOR=0``. This will save ~1KB RAM and ~3KB of flash
   memory.


API Documentation
-----------------

.. doxygengroup:: commandhandler
   :content-only:
