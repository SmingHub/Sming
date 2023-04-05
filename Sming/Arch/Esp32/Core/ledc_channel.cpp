#ifndef LEDC_CHANNEL_H
#include "ledc_channel.h"
ledc_channel::ledc_channel(ledc_mode_t mode, int gpio, ledc_timer_t timer, uint32_t duty, int hpoint){
    periph_module_enable(PERIPH_LEDC_MODULE);
    channel_config.speed_mode   =   mode;
    channel_config.gpio_num     =   gpio;
    channel_config.channel      =   Channel::instance()->getChannel(mode);
    channel_config.intr_type    =   LEDC_INTR_DISABLE;
    channel_config.timer_sel    =   timer;
    channel_config.duty         =   duty;
    channel_config.hpoint       =   hpoint;
    
    esp_err_t err=ledc_channel_config(&channel_config);
    if(err==ESP_OK){
        bindChannelTimer(timer);
    }
}

ledc_channel::ledc_channel(ledc_mode_t mode, int gpio, ledc_timer_t timer, uint32_t duty){
    ledc_channel(mode, gpio, timer, duty, (int) 0);
}

ledc_channel::ledc_channel(ledc_mode_t mode, int gpio, ledc_timer_t timer){
    return ledc_channel(mode, gpio, timer, (uint32_t) 0, (int) 0);
}

ledc_channel::ledc_channel(ledc_mode_t mode, int gpio){
    return ledc_channel(mode, gpio, Timer::instance()->getTimer(mode), (uint32_t) 0, (int) 0);
}

ledc_channel::~ledc_channel(){
    stop((uint32_t)0);
}
/*
ledc_channel::channelConfig(const ledc_channel_config_t *ledc_conf){

}
*/
ledc_channel::updateDuty(){
    return ledc_update_duty();
}

ledc_channel::setPin(int gpio_num){
    esp_err_t err = ledc_set_pin(gpio_num, channel_config.mode, channel_config.channel);
    if(esp_err==ESP_OK){
        channel_config.gpio_num=gpio_num;
    }
    return err;
}

ledc_channel::stop(uint32_t idle_level){
    return ledc_stop(channel_config.mode, channel_config.channel, idle_level);
}

ledc_channel::setDutyWithHpoint(uint32_t duty, uint32_t hpoint){
    esp_err_t err = ledc_set_duty_with_hpoint(channel_config.mode, chanel_config.channel, duty, hpoint);
    if(err==ESP_OK){
        channel_config.duty=duty;
        channel_config.hpoint=hpoint;
    }
    return err;
}

ledc_channel::setDuty(uint32_t duty){
    esp_err_t err=ledc_set_duty(channel_config.mode, channel_config.channel, duty);
    if(err==ESP_OK){
        channel_config.duty=duty;
    }
    return err;
}

ledc_channel::bindChannelTimer(ledc_timer_t timer){
    esp_err_t err = ledc_bind_channel_timer(channel_config.mode, channel_config.channel, timer);
    if(err==ESP_OK){
        channel_config.timer=timer;
    }

}
#endif