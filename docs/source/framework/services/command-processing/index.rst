Command Executor
================

.. highlight:: c++

Introduction
------------

Command handler provides a common command line interface (CLI). Command line must be text. Commands should be separated with a single character.
CLI can be used with: 

- Serial
- Websockets

and all communication protocols that are exchanging text data.

Commands can be added to and removed from the command handler. Each command will trigger a defined Delegate.

A welcome message may be shown when a user connects and end of line character may be defined. An automatic "help" display is available.

For more examples take a look at the
:sample:`CommandProcessing_Debug`
and :sample:`HttpServer_WebSockets`
samples.

API Documentation
-----------------

.. doxygengroup:: commandhandler
   :content-only:
   :members:
