Command Processing
==================

.. highlight:: c++

Introduction
------------

Command handler provides a common command line interface (CLI). Command line must be text. Commands should be separated with a single character.
CLI can be used with: 

- Serial
- Network (Websockets, Telnet)

and all communication protocols that are exchanging text data.

Commands can be added to and removed from the command handler. Each command will trigger a defined Delegate.

A welcome message may be shown when a user connects and end-of-line (EOL) character may be defined. An automatic "help" display is available.

For more examples take a look at the
:sample:`CommandLine`,
:sample:`TelnetServer`
and :sample:`HttpServer_WebSockets`
samples.


Using
-----

1. Add these lines to your application componenent.mk file::

       COMPONENT_DEPENDS += CommandProcessing

2. Add these lines to your application::

      #include <CommandProcessing/Utils.h>

3. Basic example::

      #include <CommandProcessing/Utils.h>

      CommandProcessing::Handler commandHandler;
      
      void processShutdownCommand(String commandLine, ReadWriteStream& commandOutput)
      {
        // ...
      }

      void init()
      {
        commandHandler.registerSystemCommands();
        commandHandler.registerCommand({CMDP_STRINGS("shutdown", "Shutdown Server Command", "Application"), processShutdownCommand});
      }

.. envvar:: CMDPROC_FLASHSTRINGS

   default: undefined (RAM strings)
   1: Store strings in flash memory

   Command ``name``, ``help`` and ``group`` strings default to RAM.
   This maintains backward compatibility with existing applications which may define commands like this::

     commandHandler.registerCommand({"shutdown", "Shutdown Server Command", "Application", processShutdownCommand});

   To save RAM, update your code to use the ``CMDP_STRINGS`` macro and build with ``CMDPROC_FLASHSTRINGS=1``.


API Documentation
-----------------

.. doxygengroup:: commandhandler
   :content-only:
   :members:

