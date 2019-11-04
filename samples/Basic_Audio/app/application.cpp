/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * application.cpp
 *
 * Basic audio sample
 *
 ****/

#include <SmingCore.h>
#include <driver/i2s.h>
#include <Libraries/ToneGenerator/DeltaSigma.h>
#include <Services/Profiling/MinMaxTimes.h>
#include <SignalGenerator.h>

// If using an external DAC, set this to 0 - you'll probably need to change other settings as well
#define ENABLE_DELTA_SIGMA

// Set this to 0 to output fixed values for easier checking on a scope or signal analyzer
//#define GENERATE_FIXED_VALUES
constexpr i2s_sample_t fixedSampleValue = {0xF55F0AA0};

// I2S sample rate to request from hardware
constexpr unsigned targetSampleRate = 44100;

// Target frequency to generate
constexpr float sineWaveFrequency = 440;

// Measure time taken to fill the I2S DMA buffers
static Profiling::MicroTimes fillTime("Fill Time");

// Measure time taken between I2S callback (interrupt) and our task callback getting executed
static Profiling::MicroTimes callbackLatency("Callback latency");

// Report status periodically
static SimpleTimer statusTimer;
static constexpr unsigned statusIntervalMs = 5000;

// One full sine-wave cycle
static struct {
	uint16_t* samples = nullptr;
	unsigned sampleCount = 0;
	unsigned readPos = 0;

	bool generate(float sampleRate, float frequency)
	{
		delete samples;
		readPos = 0;

		sampleCount = round(sampleRate / frequency);
		// Modify frequency to fit in exact number of samples
		frequency = sampleRate / sampleCount;

		Serial.print("Generating sine wave table @ ");
		Serial.print(frequency);
		Serial.print(" Hz, ");
		Serial.print(sampleCount);
		Serial.println(" samples");

		samples = new uint16_t[sampleCount];
		if(samples == nullptr) {
			debug_e("Memory allocation failed");
			return false;
		}

		SignalGenerator gen(eST_Sine, frequency);
		for(unsigned i = 0; i < sampleCount; ++i) {
			float t = i / (sampleCount * frequency);
			float value = gen.getValue(t);
			samples[i] = round(value * 32767);
		}

		return true;
	}

	uint16_t read()
	{
		uint16_t value = samples[readPos++];
		if(readPos == sampleCount) {
			readPos = 0;
		}
		return value;
	}
} sineWaveTable;

/*
 * Outputs a 172.266Hz sine wave (256 samples at 44100 samples/sec)
 */
void writeSine()
{
	unsigned frames_written = 0;
	i2s_buffer_info_t info;
	while(i2s_dma_write(&info, UINT_MAX)) {
		unsigned frame_count = info.size / sizeof(i2s_sample_t);
		for(unsigned i = 0; i < frame_count; ++i) {
			uint32_t sample = sineWaveTable.read();
#ifdef ENABLE_DELTA_SIGMA
			static DeltaSigma mod;
			sample = mod.update(sample);
#endif
			info.samples[i].u32 = sample;
		}

		frames_written += frame_count;
	}
}

void writeFixedValues()
{
	i2s_buffer_info_t info;
	while(i2s_dma_write(&info, UINT_MAX)) {
		memset(info.samples, 0, info.size);
		//		for(unsigned i = 0; i < info.size / sizeof(i2s_sample_t); ++i) {
		//			info.samples[i] = fixedSampleValue;
		//		}
	}
}

void fillBuffers()
{
	callbackLatency.update();
	fillTime.start();
#ifdef GENERATE_FIXED_VALUES
	writeFixedValues();
#else
	writeSine();
#endif
	fillTime.update();
}

static void checkReceive()
{
	unsigned total = 0;
	i2s_buffer_info_t info;
	while(i2s_dma_read(&info, UINT_MAX)) {
		total += info.size;
		m_printHex("DATA", info.buffer, info.size);
	}
	debug_i("RX: %u bytes", total);
}

void IRAM_ATTR i2sCallback(void* param, i2s_event_type_t event)
{
	// For this sample, process the data in task context
	switch(event) {
	case I2S_EVENT_TX_DONE:
		System.queueCallback(fillBuffers);
		callbackLatency.start();
		break;
	case I2S_EVENT_RX_DONE:
		System.queueCallback(checkReceive);
		break;
	default:; // ignore
	}
}

static void initialiseI2S()
{
	i2s_config_t config;
	memset(&config, 0, sizeof(config));
	const i2s_module_config_t modcfg = {
		.mode = I2S_MODE_MASTER,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		.dma_buf_len = 128,
		.dma_buf_count = 4,
		.callback_threshold = 1,
	};

	/*
	 * Not clear exactly what this setting does.
	 * I think, additional bits written out.
	 *
	 * For example, 16-bit channel data + bits_mod=8 produces 24-bits per channel.
	 * Where do additional 8 bits come from? Scope says 0.
	 */
	config.bits_mod = 16;

	config.tx = modcfg;
	config.sample_rate = targetSampleRate;
	//	config.rx = modcfg;
	//	config.rx.mode = I2S_MODE_SLAVE;
	//	config.tx_desc_auto_clear = true;
	//	config.auto_start = true;
	config.callback = i2sCallback;
	config.param = nullptr; // If you want to pass a parameter to the callback routine
	if(!i2s_driver_install(&config)) {
		debug_e("i2s_driver_install failed");
		return;
	}

#ifdef ENABLE_DELTA_SIGMA
	// We're doing delta mod so only need one pin
	i2s_set_pins(I2S_PIN_DATA_OUT, true);
#else
	// Real I2S DACs require channel select (assuming its stereo) and clock lines
	i2s_set_pins(I2S_PIN_DATA_OUT | I2S_PIN_WS_OUT | I2S_PIN_BCK_OUT, true);
#endif

	auto realSampleRate = i2s_get_real_rate();
	Serial.print(_F("I2S initialised, rate = "));
	Serial.println(realSampleRate);

#ifndef GENERATE_FIXED_VALUES
	/*
	 * Generate sine wave data using the actual sample rate, which may be
	 * different to that requested.
	 */
	if(!sineWaveTable.generate(realSampleRate, sineWaveFrequency)) {
		return;
	}
#endif

	// Pre-fill the buffers
	fillBuffers();

	// Schedule a periodic status report
	statusTimer
		.initializeMs<statusIntervalMs>([]() {
			Serial.println(fillTime);
			fillTime.clear();
			Serial.println(callbackLatency);
			callbackLatency.clear();
		})
		.start();

	// Test receive
	//		i2s_enable_loopback(true);

	// and away we go
	i2s_start();
}

void init()
{
	/*
	 * Primary serial port pins are used by I2S.
	 * We're using the alternate pin mappings - see Serial.swap() below.
	 * Could use the second (output-only) UART instead
	 */
	//	Serial.setPort(UART_ID_1);

	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	// See above - must call swap after port is initialised
	Serial.swap();

	// Network not required for this sample
	WifiStation.enable(false, false);
	WifiAccessPoint.enable(false, false);

	// We could initialise I2S directly here, but not in any rush
	System.onReady(initialiseI2S);
}
