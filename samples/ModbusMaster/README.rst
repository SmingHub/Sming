ModbusMaster Library
====================

This library handles Modbus master RTU communication. The uses pre- 
and post-transmission callbacks to drive RS485 interface and to provide 
some debug information.

The original author of the library is 4-20ma:
https://github.com/4-20ma/ModbusMaster/

The one included in Sming is nomis' fork:
https://github.com/nomis/ModbusMaster (see branch fixes-2.0.1)
that isn't yet merged to the original repository.

The submodule target url points to kmihaylov's repository in case some patching 
is needed to make it work in Sming.
