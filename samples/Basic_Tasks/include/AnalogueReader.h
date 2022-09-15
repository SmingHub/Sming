#pragma once

#include <Services/Profiling/MinMaxTimes.h>
#include <Platform/Timers.h>
#include <Task.h>
#include <array>
#include <numeric>
#include <arduinoFFT.h>

/**
 *
 * @tparam sampleCount Must be a power of 2. Buckets are: 0.1  0.2  0.5  1K   2K   4K
 * @tparam samplingFrequency In Hz, must be 10000 or less due to ADC conversion time
 * Determines maximum frequency that can be analysed by the FFT.
 * @tparam reportIntervalMs
 */
template <unsigned sampleCount = 512, unsigned samplingFrequency = 10000, unsigned reportIntervalMs = 3000>
class AnalogueReader : public Task
{
public:
	AnalogueReader() : loopTimes("Loop Times")
	{
		printTimer.reset<reportIntervalMs>();
	}

	void loop() override;

	void onNotify(Notify code) override;

private:
	void processSamples();

	// Timing information in clock ticks
	ElapseTimer sampleTimer;
	uint32_t sampleIntervalTicks{0}; ///< Ticks between samples
	uint32_t groupStartTicks{0};
	uint32_t nextSampleTicks{0};
	unsigned sampleIndex{0};
	unsigned missedSamples{0};
	bool restartSampler{true};

	std::array<double, sampleCount> vReal, vImag;

	// Stats.
	OneShotFastMs printTimer;		 ///< How often to print results
	Profiling::MilliTimes loopTimes; ///< Times how long between calls
};

#define ANALOGUE_READER(returnType)                                                                                    \
	template <unsigned sampleCount, unsigned samplingFrequency, unsigned reportIntervalMs>                             \
	returnType AnalogueReader<sampleCount, samplingFrequency, reportIntervalMs>

ANALOGUE_READER(void)::processSamples()
{
	arduinoFFT fft(vReal.data(), vImag.data(), sampleCount, samplingFrequency);
	fft.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
	fft.Compute(FFT_FORWARD);
	fft.ComplexToMagnitude();

	auto average = [this](unsigned first, unsigned last) {
		double total{0};
		total = std::accumulate(vReal.begin() + first, vReal.begin() + last, total);
		unsigned count = last - first + 1;
		return int(round(total / count));
	};

	//	for(unsigned i = 0; i < SAMPLE_COUNT; ++i) {
	//		int value = vReal[i];
	//		m_printf("[%u] = %d, ", i, value);
	//	}
	//
	//	return;

	/*
	 * Don't use sample 0 and only first SAMPLE_COUNT/2 are usable.
	 * Each array element represents a frequency and its value the amplitude.
	 *
	 * 0 - 5		0 		195
	 * 6 - 12		234
	 * 13 - 32		508
	 * 33 - 62		1290
	 * 63 - 105		2461
	 * 106 - 120	4141	4688
	 */
	int bands[6] = {average(0, 5),   average(6, 12),   average(13, 32),
					average(33, 62), average(63, 105), average(106, 120)};

	for(unsigned i = 0; i < ARRAY_SIZE(bands); ++i) {
		if(i > 0) {
			Serial.print(", ");
		}
		Serial.print(bands[i]);
	}
	Serial << " - " << missedSamples << " missed" << endl;
}

ANALOGUE_READER(void)::loop()
{
	loopTimes.update();

	auto now = sampleTimer.ticks();

	if(restartSampler) {
		// Advance group start time
		auto groupLength = sampleIntervalTicks * sampleCount;
		while(int(now - groupStartTicks) > 0) {
			groupStartTicks += groupLength;
		}
		vReal.fill(0);
		vImag.fill(0);
		nextSampleTicks = groupStartTicks;
		sampleIndex = 0;
		missedSamples = 0;
		restartSampler = false;

		// Serial << "groupStartTicks = " << groupStartTicks,
		// 	" <<  now = " << now << ", diff = " << int(now - groupStartTicks) << endl;

	} else if(int(nextSampleTicks - now) <= 0) {
		unsigned index = (now - groupStartTicks) / sampleIntervalTicks;
		missedSamples += min(index, sampleCount) - sampleIndex;
		if(index < sampleCount) {
			vReal[index] = analogRead(A0);
			sampleIndex = index + 1;
		}

		if((index + 1) >= sampleCount) {
			processSamples();

			restartSampler = true;
		} else {
			do {
				nextSampleTicks += sampleIntervalTicks;
			} while(int(now - nextSampleTicks) > 0);
		}
	}

	if(printTimer.expired()) {
		Serial.println(loopTimes);
		loopTimes.clear();
		printTimer.start();

		//		sleep(1500);
	} else {
		loopTimes.start();
	}
}

ANALOGUE_READER(void)::onNotify(Notify code)
{
	switch(code) {
	case Notify::Waking:
	case Notify::Resuming:
		loopTimes.start();
		loopTimes.clear();
		sampleIntervalTicks = sampleTimer.timeToTicks(1000000 / samplingFrequency);
		groupStartTicks = sampleTimer.ticks();
		restartSampler = true;

		Serial << _F("sampleIntervalTicks = ") << sampleIntervalTicks << endl;
		break;
	default:;
	}
}

#undef ANALOGUE_READER
