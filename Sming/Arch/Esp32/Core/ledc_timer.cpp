#ifndef LEDC_TIMER_H
#include "ledc_timer.h"
#define LEDC_TIMER_H
ledc_timer::ledc_timer(ledc_mode_t mode, ledc_timer_bit_t duty_resolution, ledc_timer_bit_t bit_num, uint32_t freq, ledc_clk_cfg_t clk_cfg){
    timer_conf={
        .speed_mode         =   mode,
        .timer_num          =   Timer::instance()->getTimer(mode),
        .duty_resolution    =   duty_resolution,
        .bit_num            =   bit_num,
        .freq_hz            =   freq,
        .clk_cfg            =   clk_cfg
    };
    return ledc_timer_config(&timer_conf);
}

ledc_timer::~ledc_timer(void){
    ledc_timer_pause(timer_conf.speed_mode, timer_conf.timer_num);
    Timer::instance()->freeTimer(timer_conf.speed_mode, timer_conf.timer_num);
}

ledc_timer::setTimer(uint32_t clock_divider, uint32_t clock_div, uint32_t duty_resolution, ledc_clk_src_t clk_src){
    esp_err_t err=ledc_timer_set(
        timer_conf.speed_mode,
        timer_conf.timer_num,
        clock_div,
        duty_resolution,
        clk_src
    );
    if(err==ESP_OK){
        timer_conf={
            .duty_resolution    = duty_resolution,
            .clk_src            = clk_src
        };
        clock_divider=clock_div;
    }
    return err;
}

ledc_timer::setTimerFrequency(uint32_t freq){
    esp_err_t err=ledc_set_freq(
        timer_conf.speed_mode,
        timer_conf.timer_num,
        freq
    );
    if(err==ESP_OK){
        timer_conf.freq_hz      = freq;
    }
    return err;
}

ledc_timer::timerReset(void){
    return ledc_timer_rst(timer_conf.speed_mode, timer_conf.timer_num);
}

ledc_timer::timerPause(void){
    return ledc_timer_pause(timer_conf.speed_mode, timer_conf.timer_num);
}

ledc_timer::timerResume(void){
    return ledc_timer_resume(timer_conf.speed_mode, timer_conf.timer_num);
}
/*
ledc_timer::timerBindChannel(ledc_channel_t channel){
    esp_err_t err=ledc_bind_channel_timer(timer_conf.speed_mode, channel, timer_conf.timer_num);
    if(err==ESP_OK)
}
*/
ledc_timer::getTimerNumber(void){
    return timer_conf.timer_num;
}

ledc_timer::getTimerMode(void){
    return timer_conf.speed_mode;
}

ledc_timer::getDutyResolution(void){
    return timer_conf.duty_resolution;
}

getTimerFrequency(void){
    return timer_conf.freq_hz;
}

ledc_timer::getClockDivider(void){
    return clock_divider;
}