/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ToneGenerator.h - Tone generation via I2S
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "../ToneBufferQueue.h"
#include <WString.h>
#include <driver/i2s.h>

#define TG_VOICE_MAP(XX)                                                                                              \
	XX(Sine)                                                                                                           \
	XX(Triangular)                                                                                                     \
	XX(Sawtooth)                                                                                                       \
	XX(Square)

#define TG_EFFECT_MAP(XX)                                                                                            \
	XX(Normal)                                                                                                         \
	XX(Mute)                                                                                                           \
	XX(FadeIn)                                                                                                         \
	XX(FadeOut)

/**
 * @brief Generates tones with smooth transitions using Tone Buffers
 * @note During tone playback, a single `active` ToneBuffer feeds the I2S with samples.
 * When a new tone is prepared via `queueTone()`, a ToneBuffer is added to the
 * `pending` queue. If transitioning between tone and silence then additional
 * fade in/out buffers are queued to implement low-pass filtering on these relatively
 * abrupt transitions and thus reduce clicking.
 * Calling `submitPending()` appends the `pending` queue onto the `transition` queue.
 * The transition to the new tone (or silence) is made within `i2sWrite()`,
 * at which point buffers are released back to the `avail` queue.
 */
class ToneGenerator
{
public:
	enum class Voice {
#define XX(t) t,
		TG_VOICE_MAP(XX)
#undef XX
			MAX
	};

	enum class ToneEffect {
#define XX(t) t,
		TG_EFFECT_MAP(XX)
#undef XX
	};

	static String getVoiceName(Voice voice);
	static String getEffectName(ToneEffect effect);

	~ToneGenerator()
	{
		end();
	}

	/**
	 * @brief Initialise the tone generator and I2S
	 * @param sampleRate
	 * @retval bool true on success
	 */
	bool begin(unsigned sampleRate);

	/**
	 * @brief Stop playback and un-initialise I2S
	 * @note Releases all allocated memory
	 */
	void end();

	/**
	 * @brief Start tone playback
	 */
	bool start();

	/**
	 * @brief Stop tone playback and release any memory allocated for buffers
	 * @note Leaves I2S initialised. Call `start()` to resume.
	 */
	void stop();

	bool isStarted()
	{
		return started;
	}

	/**
	 * @brief Create a tone and queue it
	 * @param voice Voice to use for tone
	 * @param frequency Frequency in Hz
	 * @param effect
	 * @param repeatCount Specify non-zero to repeat this tone a specific number of times
	 * @retval ToneBuffer* The queued buffer, nullptr on error
	 */
	ToneBuffer* createTone(Voice voice, unsigned frequency, ToneEffect effect, unsigned repeatCount = 0);

	/**
	 * @brief Create a tone with appropriate filtering
	 * @param voice Voice to use for tone
	 * @frequency Frequency in Hz
	 * @
	 */
	void queueTone(Voice voice, unsigned frequency);

	/**
	 * @brief Submit queued tone buffers for playback
	 */
	void submitPending()
	{
		transition.append(pending);
	}

private:
	ToneBuffer* allocateBuffer(unsigned sampleCount);
	int16_t getSampleValue(Voice voice, unsigned pos, unsigned sampleCount, int16_t amplitude);
	static void IRAM_ATTR i2sCallback(void* param, i2s_event_type_t event);
	void IRAM_ATTR i2sWrite();
	bool IRAM_ATTR i2sWriteBuffer(ToneBuffer* buffer, bool end);

private:
	unsigned sampleRate;
	unsigned curFreq = 0;
	static constexpr uint8_t fadeCycles = 2;
	static constexpr uint8_t noteGapMs = 16;
	// Three buffer queues
	ToneBuffer* active = nullptr;
	ToneBufferQueue pending;
	ToneBufferQueue transition;
	ToneBufferQueue unused;
	unsigned offset = 0; // Read position in active buffer
	bool started = false;
};
