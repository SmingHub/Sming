#ifndef LEDC_TIMER_H
#define LEDC_TIMER_H
#include <debug_progmem.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "singleton.h"
#include <hal/ledc_types.h>

#ifndef LEDC_CHANNEL_H
#include "ledc_channel.h"
#endif

class ledc_timer {
    public:
        ledc_timer(ledc_mode_t mode, ledc_timer_bit_t duty_resolution, uint32_t freq, ledc_clk_cfg_t clk_cfg);
        ~ledc_timer(void);
        esp_err_t setTimer(uint32_t clock_divider, ledc_timer_bit_t duty_resolution, ledc_clk_src_t clk_src);
        esp_err_t setTimerFrequency(uint32_t freq);
        esp_err_t timerReset(void);
        esp_err_t timerPause(void);
        esp_err_t timerResume(void);
        esp_err_t timerBindChannel(ledc_channel_t channel);

        ledc_timer_t getTimerNumber(void);
        ledc_mode_t getTimerMode(void);

        ledc_timer_bit_t getDutyResolution(void);        
        uint32_t getTimerFrequency(void);
        uint32_t getClockDivider(void);
    private:
        ledc_timer_config_t timer_conf;
        ledc_timer_t timer;
        uint32_t clock_divider;
};

#endif