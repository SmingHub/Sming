#include <SmingCore.h>
#include <Graphics/SampleConfig.h>
#include <AnimatedGifTask.h>

namespace
{
AnimatedGifTask* task;

IMPORT_FSTR(gifData, PROJECT_DIR "/files/frog.gif")

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	initDisplay();
	tft.setOrientation(Graphics::Orientation::deg270);

	task = new AnimatedGifTask(tft.createSurface(), (uint8_t*)gifData.data(), gifData.length(), true);
	task->resume();
}
