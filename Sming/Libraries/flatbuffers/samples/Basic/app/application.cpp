#include <SmingCore.h>

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
	builder.Finish(orc); // You could also call `FinishMonsterBuffer(builder,
    					//                                          orc);`.


	// and then decode it
	uint8_t *buffer = builder.GetBufferPointer();
	auto monster = GetMonster(buffer);
	Serial.printf("Monster name: %s\n", monster->name()->c_str());
}
