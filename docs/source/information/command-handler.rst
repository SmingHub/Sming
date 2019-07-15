***************
Command Handler
***************

Summary
=======

The Sming ``CommandHandler`` implementation adds the possibility to
handle system and application commands over a variety of input media.

The CLI provides a framework in which basic system commands can be
extended and application specific commands created.

Currently implemented are: - Serial - Telnet Server - Websocket Server

Implementation
==============

The implementation makes use of a global object ``CommandHandler``,
which at the start is just a skeleton without any command active.

System commands can be activated using: -
``Serial.commandProcessing(true)`` -
``telnetServer.enableCommand(true)`` -
``websockerServer.commandProcessing(true, "command")``

The additional parameter for websocket is used to allow multiple
connections on one websocket server instance, both with or without
command proceesing.

When the websocket open request has query parameter ``"command=true"``
command processing is enabled.

Usage
=====

The usage of ``CommandProcessor`` is the same for all implementations,
except for specific requirements. ``CommandProcesor`` options
implemented are: - Verbose -> sets display of welcome message - Prompt
-> set prompt string - EOL -> set end of line character - welcome
message -> set the welcome message

System Commands
===============

The base ``CommandHandler`` has the following sysytem commands
implemented: - ``help``, Displays all available commands - ``status``,
Displays System Information - ``echo``, Displays command entered -
``debugon``, Set Serial debug on - ``debugoff``, Set Serial debug off -
``command``, Usage verbose/silent/prompt for command options

Application options
===================

Applications (and Sming Core functionality) can add commands to the
``CommandProcesor`` using a construct like

.. code-block:: c++

   registerCommand(CommandDelegate(
       "status",
       "Displays System Information",
       "system",
       commandFunctionDelegate(&CommandHandler::procesStatusCommand, this)
   ));

The added command will then be available over every opened command
channel.

Example usage
=============

The capabilities of ``CommandHandler`` are shown in the example
``CommandProcessing_Debug``.

The example will create an application which shows ``CommandProcessing``
for Telnet, Websocket and Serial.

-  to test Telnet, open telnet client and connect on port 23 to the ESP
   ip
-  to test the Websocket, open a web browser with the ESP
-  to test Serial use your “Serial program” and make sure “local echo”
   is activated

For all implementations type “help” to show the available Commands

The possibilities of extending commands with the application are shown
in: - the class ``ExampleCommand`` - the functions
``startExampleApplicationCommand()`` and
``processApplicationCommands()``
