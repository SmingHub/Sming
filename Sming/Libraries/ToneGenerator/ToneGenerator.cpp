/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ToneGenerator.cpp
 *
 ****/

#include "include/ToneGenerator.h"
#include <string.h>
#include "DeltaSigma.h"
#include <algorithm>
#include <FakePgmSpace.h>
#include <stringutil.h>
#include <Data/CStringArray.h>

/*
 * Lookup table for quarter sine wave

	#include <cmath>
	#include <WConstants.h>

	Serial.println();
	for(unsigned i = 0; i < 512; ++i) {
		if(i % 8 == 0) {
			Serial.println();
		}
		double value = sin(HALF_PI * i / 512);
		int sample = round(65535 * value);
		Serial.print(sample);
		Serial.print(", ");
	}
	Serial.println();

 */
static const uint16_t PROGMEM sineTable[] = {
	0,	 201,   402,   603,   804,   1005,  1206,  1407,  1608,  1809,  2010,  2211,  2412,  2613,  2814,  3015,
	3216,  3416,  3617,  3818,  4019,  4219,  4420,  4621,  4821,  5022,  5222,  5422,  5623,  5823,  6023,  6223,
	6424,  6624,  6824,  7024,  7223,  7423,  7623,  7823,  8022,  8222,  8421,  8620,  8820,  9019,  9218,  9417,
	9616,  9815,  10014, 10212, 10411, 10609, 10808, 11006, 11204, 11402, 11600, 11798, 11996, 12193, 12391, 12588,
	12785, 12982, 13179, 13376, 13573, 13770, 13966, 14163, 14359, 14555, 14751, 14947, 15142, 15338, 15533, 15729,
	15924, 16119, 16313, 16508, 16703, 16897, 17091, 17285, 17479, 17673, 17866, 18060, 18253, 18446, 18639, 18831,
	19024, 19216, 19408, 19600, 19792, 19984, 20175, 20366, 20557, 20748, 20939, 21129, 21319, 21509, 21699, 21889,
	22078, 22267, 22456, 22645, 22834, 23022, 23210, 23398, 23586, 23773, 23960, 24147, 24334, 24521, 24707, 24893,
	25079, 25265, 25450, 25635, 25820, 26005, 26189, 26374, 26557, 26741, 26925, 27108, 27291, 27473, 27656, 27838,
	28020, 28201, 28383, 28564, 28745, 28925, 29106, 29286, 29465, 29645, 29824, 30003, 30181, 30360, 30538, 30716,
	30893, 31070, 31247, 31424, 31600, 31776, 31952, 32127, 32302, 32477, 32651, 32826, 32999, 33173, 33346, 33519,
	33692, 33864, 34036, 34208, 34379, 34550, 34721, 34891, 35061, 35231, 35400, 35569, 35738, 35906, 36074, 36242,
	36409, 36576, 36743, 36909, 37075, 37241, 37406, 37571, 37736, 37900, 38064, 38227, 38390, 38553, 38715, 38877,
	39039, 39200, 39361, 39522, 39682, 39842, 40001, 40161, 40319, 40478, 40635, 40793, 40950, 41107, 41263, 41419,
	41575, 41730, 41885, 42039, 42194, 42347, 42500, 42653, 42806, 42958, 43109, 43261, 43411, 43562, 43712, 43861,
	44011, 44159, 44308, 44456, 44603, 44750, 44897, 45043, 45189, 45334, 45479, 45624, 45768, 45912, 46055, 46198,
	46340, 46482, 46624, 46765, 46905, 47046, 47185, 47325, 47464, 47602, 47740, 47877, 48014, 48151, 48287, 48423,
	48558, 48693, 48827, 48961, 49095, 49228, 49360, 49492, 49624, 49755, 49885, 50016, 50145, 50274, 50403, 50531,
	50659, 50787, 50913, 51040, 51166, 51291, 51416, 51540, 51664, 51788, 51911, 52033, 52155, 52277, 52398, 52518,
	52638, 52758, 52877, 52995, 53113, 53231, 53348, 53464, 53580, 53696, 53811, 53925, 54039, 54153, 54266, 54378,
	54490, 54602, 54713, 54823, 54933, 55042, 55151, 55260, 55367, 55475, 55582, 55688, 55794, 55899, 56003, 56108,
	56211, 56314, 56417, 56519, 56620, 56721, 56822, 56922, 57021, 57120, 57218, 57316, 57413, 57510, 57606, 57702,
	57797, 57891, 57985, 58079, 58171, 58264, 58356, 58447, 58537, 58628, 58717, 58806, 58895, 58983, 59070, 59157,
	59243, 59329, 59414, 59498, 59582, 59666, 59749, 59831, 59913, 59994, 60075, 60155, 60234, 60313, 60391, 60469,
	60546, 60623, 60699, 60775, 60850, 60924, 60998, 61071, 61144, 61216, 61287, 61358, 61429, 61498, 61567, 61636,
	61704, 61772, 61838, 61905, 61970, 62035, 62100, 62164, 62227, 62290, 62352, 62414, 62475, 62535, 62595, 62654,
	62713, 62771, 62829, 62886, 62942, 62998, 63053, 63107, 63161, 63214, 63267, 63319, 63371, 63422, 63472, 63522,
	63571, 63620, 63668, 63715, 63762, 63808, 63853, 63898, 63943, 63986, 64030, 64072, 64114, 64155, 64196, 64236,
	64276, 64315, 64353, 64391, 64428, 64464, 64500, 64535, 64570, 64604, 64638, 64671, 64703, 64734, 64765, 64796,
	64826, 64855, 64883, 64911, 64939, 64966, 64992, 65017, 65042, 65066, 65090, 65113, 65136, 65158, 65179, 65199,
	65219, 65239, 65258, 65276, 65293, 65310, 65327, 65342, 65357, 65372, 65386, 65399, 65412, 65424, 65435, 65446,
	65456, 65466, 65475, 65483, 65491, 65498, 65504, 65510, 65515, 65520, 65524, 65527, 65530, 65532, 65534, 65535,
};

String ToneGenerator::getVoiceName(Voice voice)
{
#define XX(t) #t "\0"
	DEFINE_FSTR_LOCAL(names, TG_VOICE_MAP(XX));
#undef XX
	return CStringArray(names)[unsigned(voice)];
}

String ToneGenerator::getEffectName(ToneEffect effect)
{
#define XX(t) #t "\0"
	DEFINE_FSTR_LOCAL(names, TG_EFFECT_MAP(XX));
#undef XX
	return CStringArray(names)[unsigned(effect)];
}

void ToneGenerator::i2sCallback(void* param, i2s_event_type_t event)
{
	if(event == I2S_EVENT_TX_DONE) {
		static_cast<ToneGenerator*>(param)->i2sWrite();
	}
}

bool ToneGenerator::begin(unsigned sampleRate)
{
	end();
	this->sampleRate = sampleRate;

	i2s_config_t config;
	memset(&config, 0, sizeof(config));
	const i2s_module_config_t modcfg = {
		.mode = I2S_MODE_MASTER,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		.dma_buf_len = 512,
		.dma_buf_count = 2,
		.callback_threshold = 1,
	};

	config.tx = modcfg;
	config.sample_rate = sampleRate;
	config.callback = i2sCallback;
	config.param = this;
	if(!i2s_driver_install(&config)) {
		return false;
	}

	i2s_set_pins(I2S_PIN_DATA_OUT, true);
	return true;
}

void ToneGenerator::end()
{
	stop();
	i2s_driver_uninstall();
}

bool ToneGenerator::start()
{
	if(!started) {
		started = i2s_start();
	}
	return started;
}

void ToneGenerator::stop()
{
	if(started) {
		i2s_stop();
		delete active;
		active = nullptr;
		pending.clear();
		transition.clear();
		unused.clear();
		started = false;
	}
}

ToneBuffer* ToneGenerator::allocateBuffer(unsigned sampleCount)
{
	auto buffer = unused.dequeue();
	if(buffer == nullptr) {
		buffer = new ToneBuffer;
		if(buffer == nullptr) {
			return nullptr;
		}
	}

	if(!buffer->allocate(sampleCount)) {
		delete buffer;
		return nullptr;
	}

	return buffer;
}

static int getSineValue(unsigned pos, unsigned sampleCount)
{
	unsigned i = 4 * ARRAY_SIZE(sineTable) * pos / sampleCount;
	unsigned offset = i % ARRAY_SIZE(sineTable);
	unsigned quadrant = i / ARRAY_SIZE(sineTable);
	switch(quadrant) {
	case 0:
		return -pgm_read_word(&sineTable[offset]);
	case 1:
		return -pgm_read_word(&sineTable[ARRAY_SIZE(sineTable) - offset - 1]);
	case 3:
		return pgm_read_word(&sineTable[ARRAY_SIZE(sineTable) - offset - 1]);
	case 2:
	default:
		return pgm_read_word(&sineTable[offset]);
	}
}

int16_t ToneGenerator::getSampleValue(Voice voice, unsigned pos, unsigned sampleCount, int16_t amplitude)
{
	switch(voice) {
	case Voice::Square:
		return (pos <= (sampleCount / 2)) ? -amplitude : amplitude;

	case Voice::Sawtooth:
		return (2 * amplitude * pos / sampleCount) - amplitude;

	case Voice::Triangular: {
		pos = (pos + (sampleCount / 4)) % sampleCount;
		unsigned sampleCount2 = sampleCount / 2;
		if(pos < sampleCount2) {
			return (2 * amplitude * pos / sampleCount2) - amplitude;
		} else {
			return (2 * amplitude * (sampleCount - pos - 1) / sampleCount2) - amplitude;
		}
	}

	case Voice::Sine:
	default:
		return amplitude * getSineValue(pos, sampleCount) / 65535;
	}
}

ToneBuffer* ToneGenerator::createTone(Voice voice, unsigned frequency, ToneEffect effect, unsigned repeatCount)
{
	if(frequency == 0) {
		return nullptr;
	}

	frequency = std::max(frequency, 100U);
	frequency = std::min(frequency, sampleRate / 2);

	unsigned samplesPerCycle = sampleRate / frequency;
	unsigned cycleCount = (effect < ToneEffect::FadeIn) ? 1 : fadeCycles;
	auto sampleCount = cycleCount * samplesPerCycle;

	auto buffer = allocateBuffer(sampleCount);
	if(buffer == nullptr) {
		return nullptr;
	}

	buffer->repeatCount = repeatCount;
	auto data = buffer->getData();

	if(effect == ToneEffect::Mute) {
		memset(data, 0x55, sampleCount * buffer->sampleSize);
		return buffer;
	}

	DeltaSigma mod;
	if(effect < ToneEffect::FadeIn) {
		for(unsigned i = 0; i < sampleCount; ++i) {
			int16_t sample = getSampleValue(voice, i, sampleCount, INT16_MAX);
			data[i] = mod.update(sample);
		}
	} else {
		int16_t amplitude = INT16_MAX;
		for(unsigned i = 0; i < sampleCount; ++i) {
			if(effect == ToneEffect::FadeIn) {
				amplitude = INT16_MAX * (i + 1) / sampleCount;
			} else if(effect == ToneEffect::FadeOut) {
				amplitude = INT16_MAX * (sampleCount - i) / sampleCount;
			}

			int16_t sample = getSampleValue(voice, i % samplesPerCycle, samplesPerCycle, amplitude);
			data[i] = mod.update(sample);
		}
	}

	return buffer;
}

void ToneGenerator::queueTone(Voice voice, unsigned frequency)
{
	// Filter to reduce clicking between notes
	if(frequency == 0) {
		if(curFreq != 0) {
			pending.append(createTone(voice, curFreq, ToneEffect::FadeOut));
		}
		pending.append(createTone(voice, curFreq, ToneEffect::Mute));
	} else {
		if(frequency == curFreq) {
			// Insert a small gap to separate identical notes
			pending.append(createTone(voice, curFreq, ToneEffect::FadeOut));
			pending.append(createTone(voice, curFreq, ToneEffect::Mute, noteGapMs * frequency / 1000U));
			pending.append(createTone(voice, frequency, ToneEffect::FadeIn));
		} else if(curFreq == 0) {
			pending.append(createTone(voice, frequency, ToneEffect::FadeIn));
		}
		pending.append(createTone(voice, frequency, ToneEffect::Normal));
	}
	curFreq = frequency;
}

bool ToneGenerator::i2sWriteBuffer(ToneBuffer* buffer, bool end)
{
	auto data = buffer->getData();
	auto sampleCount = buffer->getSampleCount();
	i2s_buffer_info_t info;
	while(i2s_dma_write(&info, (sampleCount - offset) * buffer->sampleSize)) {
		memcpy(info.samples, &data[offset], info.size);
		offset += info.size / buffer->sampleSize;
		if(offset != sampleCount) {
			continue;
		}

		offset = 0;
		if(buffer->repeatCount > 0) {
			--buffer->repeatCount;
		} else if(end) {
			return true;
		}
	}

	return false;
}

void ToneGenerator::i2sWrite()
{
	if(active == nullptr) {
		active = transition.dequeue();
		if(active == nullptr) {
			return;
		}
	}

	while(!transition.empty()) {
		// Transition to new buffer at zero-crossing point
		if(!i2sWriteBuffer(active, true)) {
			return;
		}
		unused.insert(active);
		active = transition.dequeue();
	}

	i2sWriteBuffer(active, false);
}
