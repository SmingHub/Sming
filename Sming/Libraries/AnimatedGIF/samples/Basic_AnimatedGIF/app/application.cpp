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

	auto surface = tft.createSurface();
	assert(surface != nullptr);
	task = new AnimatedGifTask(*surface, gifData);
	task->resume();
}
