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
:sample:`CommandProcessing`,
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
		commandHandler.registerCommand(CommandProcessing::Command("shutdown", "Shutdown Server Command", "Application", processShutdownCommand));
      }

API Documentation
-----------------

.. doxygengroup:: commandhandler
   :content-only:
   :members:

