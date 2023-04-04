#include "./singleton.h"
/**
 * @namespace HardwarePWM - stuffing away my HardwarePWM specific functions in a separate Namespace
 * 
*/
namespace HardwarePWM
{
/**
     * @brief get a pwm timer
     * 
     * @return timer number 
     * 
    */

ledc_timer_t Timer::getTimer(ledc_mode_t mode)
{
	for(uint8_t i = 0; i < (uint8_t)LEDC_TIMER_MAX; i++) {
		if(!isUsed[(uint8_t)mode][i]) {
			isUsed[(uint8_t)mode][i] = true;
			return (ledc_timer_t)i;
		}
	}
	return (ledc_timer_t)LEDC_TIMER_MAX;
}

void Timer::freeTimer(ledc_mode_t mode, ledc_timer_t timer)
{
	isUsed[(uint8_t)mode][(uint8_t)timer] = false;
}

Timer::Timer()
{
	for(uint8_t i = 0; i < (uint8_t)LEDC_SPEED_MODE_MAX; i++) {
		for(uint8_t j = 0; j < (uint8_t)LEDC_TIMER_MAX; j++) {
			isUsed[i][j] = false;
		}
	}
}

ledc_channel_t Channel::getChannel(ledc_mode_t mode)
{
	for(uint8_t i = 0; i < (uint8_t)LEDC_CHANNEL_MAX; i++) {
		if(!isUsed[(uint8_t)mode][i]) {
			isUsed[(uint8_t)mode][i] = true;
			return (ledc_channel_t)i;
		}
	}
	return (ledc_channel_t)LEDC_CHANNEL_MAX;
}

void Channel::freeChannel(ledc_mode_t mode, ledc_channel_t channel)
{
	isUsed[(uint8_t)mode][(uint8_t)channel] = false;
}

uint8_t getFreeChannels(ledc_speedmode_t mode){
	uint8_t count=0;
	for(uit8_t i=0 i<(uint8_t)LEDC_CHANNEL_MAX;i++){
		if(!isUsed[(uint8_t)mode][i]){
			count++;
		}
		return count; 
	}
}

Channel::Channel()
{
	for(int i = 0; i < LEDC_CHANNEL_MAX; i++) {
		isUsed[0][i] = false;
#ifdef SOC_LEDC_SUPPORT_HS_MODE
		isUsed[1][i] = false;
#endif
	}
}
} // end namespace HardwarePWM