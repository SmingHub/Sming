Flatbuffers
===========

.. highlight:: c++

FlatBuffers is an efficient cross platform serialization library for C++, C#, C, Go, Java, Kotlin, JavaScript, Lobster, Lua, TypeScript, PHP, Python, Rust and Swift.
It was originally created at Google for game development and other performance-critical applications.

It is available as Open Source on GitHub under the Apache license, v2 (see LICENSE.txt).

Using
-----

Step 1. Add these lines to your application componenent.mk file::

   COMPONENT_DEPENDS += flatbuffers

Step 2. Add these lines to your application::

   #include <flatbuffers/flatbuffers.h>

Or directly use the header file generated from the `flatc` compiler.

Step 3. Basic example::

   #include "monster.h"

   using namespace MyGame::Example;

   void init()
   {
      Serial.begin(SERIAL_BAUD_RATE);

      // demonstration how to encode data into a flatbuffer
      flatbuffers::FlatBufferBuilder builder;

      auto name = builder.CreateString("Sming Monster");

      MonsterBuilder monster_builder(builder);
      monster_builder.add_name(name);

      auto orc = monster_builder.Finish();

      // Call `Finish()` to instruct the builder that this monster is complete.
      // Note: Regardless of how you created the `orc`, you still need to call
      // `Finish()` on the `FlatBufferBuilder`.
      builder.Finish(orc);

      // and then decode it
      uint8_t *buffer = builder.GetBufferPointer();
      auto monster = GetMonster(buffer);
      Serial.printf("Monster name: %s\n", monster->name()->c_str());
   }

Further reading
---------------
Take a look at the `official flatbuffers tutorial <https://google.github.io/flatbuffers/flatbuffers_guide_tutorial.html>`_.
