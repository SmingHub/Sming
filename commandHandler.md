#Sming Commandline

##Summary##
The Sming Commandline implementation adds the possibility to handle System and Application Commands over a variety of input media.
The CLI provides a framework in which Basic System Commands can be extended and Application specific commands created. 

Currently implemented are :
- Serial
- Telnet Server
- Websocket Server

##Implementation##
The implementation makes use of a global object CommandHandler, which at the start is just a skeleton without any command active.
System Commands can be activated using 
`Serial.commandProcessing(true)`
`telnetServer.enableCommand(true)`
`websockerServer.commandProcessing(true,"command")`

The additional parameter for websocket is used to allow multiple connections on one websocket server instance, both with or without command proceesing
When the websocket open request has queryparameter "command=true" command processing is enabled

##usage##
The usage of CommandProcessor is the same for all imnplemenations, except for specific requirements.
Commandprocesor options implemented are :
- Verbose -> sets display of welcome message
- Prompt -> set prompt string
- EOL -> set end of line character
- welcome message -> set the welcome message

##System Commands##
The base CommandHandler has the following sysytem commands implemented :
- "help", "Displays all available commands"
- "status", "Displays System Information"
- "echo", "Displays command entered"
- "debugon", "Set Serial debug on"
- "debugoff", "Set Serial debug off"
- "command","Usage verbose/silent/prompt for command options\r\n"

##Application options##
Applications (and Sming Core functionality) can add commands to the CommandProcesor use a construct like
`registerCommand(CommandDelegate("status", "Displays System Information", "system", commandFunctionDelegate(&CommandHandler::procesStatusCommand,this)));`
The added command will then be available thru every opened command channel

##Example usage##
The capabilities of CommandHandler are shown in the exampe `Commandprocessing_Debug`