Nano Protocol-Buffer
====================

Introduction
------------

This component adds support for `Nano Protocol-Buffer <https://github.com/nanopb/nanopb/>`_ implementation.

   Nanopb is a small code-size `Protocol Buffers <https://developers.google.com/protocol-buffers>`_ implementation in ansi C. It is especially suitable for use in microcontrollers, but fits any memory restricted system.


C file generation from Proto files
----------------------------------

Once this component is installed you can use it to add Nano Protocol-Buffer support to your project and generate C and header files from Proto files.
One possible way to call the generator is to go to the directory where the proto file is located and run the generator. As shown below::

   make -C $SMING_HOME fetch nano-pb
   cd <folder-with-proto-file>
   python $SMING_HOME/Libraries/nanopb/nanopb/generator/nanopb_generator.py <desired-proto-file>.proto

After the generator tool is run you will have newly generated C and header files that can be used in your Sming application.

Using
-----

1. Add ``COMPONENT_DEPENDS += nanopb`` to your application componenent.mk file.
2. Add these lines to your application::

      #include <PbUtils.h>
      // The line below should be replaced with the generated header file
      #include "cast_channel.pb.h"

3. Example::

      #include <PbUtils.h>
      #include "cast_channel.pb.h"

      void doSomething(const uint8_t* data, size_t length)
      {
         // ...

         extensions_api_cast_channel_CastMessage message = extensions_api_cast_channel_CastMessage_init_default;

         message.protocol_version = extensions_api_cast_channel_CastMessage_ProtocolVersion_CASTV2_1_0;
         message.source_id.funcs.encode = &pbEncodeData;
         message.source_id.arg = new PbData(sourceId);
         message.destination_id.funcs.encode = &pbEncodeData;
         message.destination_id.arg = new PbData(destinationId);
         message.nameSpace.funcs.encode = &pbEncodeData;
         message.nameSpace.arg = new PbData(ns);
         message.payload_type = extensions_api_cast_channel_CastMessage_PayloadType_STRING;
         message.payload_utf8.funcs.encode = &pbEncodeData;
         message.payload_utf8.arg = new PbData((uint8_t*)data, length);
         // ...
      }