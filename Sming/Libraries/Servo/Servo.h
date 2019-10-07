/*
 * Servo.h
 *
 *  Created on: 22.11.2015
 *      Author: johndoe
 */
/** @addtogroup   Servo RC Servo functions
 *  @brief      Provides Library to control rc servos with pwm signals
 *  - uses an internal instance of HardwareTimer (which use interrupts)
 *  @{
*/

#pragma once

#include "ServoChannel.h"
#include <HardwareTimer.h>
#include <SimpleTimer.h>

/**
 * @brief Manages multiple servo channels
 * @note Do not call this class directly, instead use `ServoChannel` methods.
 */
class Servo
{
public:
	static constexpr unsigned maxChannels = 5;	 ///< maximum number of servo channels
	static constexpr uint32_t framePeriod = 20000; ///< Total frame time in microseconds
	using HardwareTimer = HardwareTimer1<TIMER_CLKDIV_16, eHWT_NonMaskable>;
	static constexpr uint32_t periodTicks = HardwareTimer::usToTicks<framePeriod>();
	//	static constexpr uint32_t minChannelTime = HardwareTimer::ticksToUs<HardwareTimer::minTicks()>();
	static constexpr uint32_t minChannelTime = MIN_HW_TIMER1_INTERVAL_US;
	//	static constexpr uint32_t maxChannelTime = HardwareTimer::ticksToUs<channelTicks - HardwareTimer::minTicks()>();
	static constexpr uint32_t maxChannelTime = framePeriod / maxChannels;

	/** @brief  Instantiate servo object
	 *  @note   Public global instance of Servo is available as variable servo.
     * 	No additional instance is allowed because ServoChannel instances use servo.
     */
	Servo();

	/** @brief  add a servo channel to the servo object
     *  @param  channel pointer to the servo channel object
     *  @retval bool True on success
     *  @note   Locks interrupt to change configuration without timing problem, may result in a short glitch
     */
	bool addChannel(ServoChannel* channel);

	/** @brief  remove a servo channel from the servo object
     *  @param  channel pointer to the servo channel object
     *  @retval bool True on success
     *  @note   Locks interrupt to change configuration without timing problem, may result in a short glitch
     */
	bool removeChannel(ServoChannel* channel);

	/** @brief  calculation of all the internal timings
     *  @note   Has to be called when min of max is changed and setDegree is used
     */
	void updateChannel(ServoChannel* channel);

	// Frame timings
	struct Frame {
		uint32_t slots[maxChannels + 1]; ///< Transition times in timer ticks
		uint8_t pins[maxChannels];		 ///< Active pins
		uint8_t slotCount = 0;
	};

	const Frame& getActiveFrame() const
	{
		return frames[activeFrameIndex];
	}

private:
	void update();
	int findChannel(ServoChannel* channel);
	void calcTiming();
	static void IRAM_ATTR staticTimerIsr();
	void IRAM_ATTR timerIsr();

private:
	ServoChannel* channels[maxChannels] = {0};
	unsigned channelCount = 0;   ///< Number of active channels
	HardwareTimer hardwareTimer; ///< Handles generation of output signals
	Frame frames[2];			 ///< Contains the active and next frames
	uint8 activeFrameIndex = 0;  ///< Frame being used by ISR
	uint8 activeSlot = 0;		 ///< Slot being output by ISR
	uint8_t nextFrameIndex = 0;  ///< Frame to use when active frame has completed
	SimpleTimer updateTimer;	 ///< If necessary, updates are pended
};

extern Servo servo; ///< global instance of the servo object
/** @} */
