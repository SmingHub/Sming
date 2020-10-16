#include <UPnP/DescriptionStream.h>
#include <Platform/Timers.h>
#include <HardwareSerial.h>

#ifdef DUMP_TO_HOST_FILE
#include <Data/Stream/HostFileStream.h>
#endif

void dumpDescription(UPnP::Object* object)
{
	Serial.println("DESCRIPTION");

	CpuCycleTimer timer;
	auto stream = new UPnP::DescriptionStream(object);
	size_t count = Serial.copyFrom(stream, UINT32_MAX);
	auto ticks = timer.elapsedTicks();
	Serial.println();

#ifdef DUMP_TO_HOST_FILE
	// Write stream to local disk within Host emulator
	stream->reset();
	HostFileStream fs(stream->getName(), eFO_CreateNewAlways | eFO_WriteOnly);
	fs.copyFrom(stream, UINT32_MAX);
#endif

	delete stream;

	Serial.print("Chars written: ");
	Serial.print(count);
	Serial.print(", elapsed: ");
	Serial.print(ticks);
	Serial.println(" ticks");
}
