#ifndef LEDC_CHANNEL_H
#include "ledc_channel.h"
ledc_channel::ledc_channel(ledc_speedmode_t mode, int gpio, ledc_timer_t timer, uint32_t duty, int hpoint){
    periph_module_enable(PERIPH_LEDC_MODULE);
    channel_config={
        .speed_mode         =   mode,
        .gpio_num           =   gpio,
        .channel            =   Channel::instance()->getChannel(mode),
        .intr_type          =   LEDC_INTR_DISABLE,
        .timer_sel          =   timer,
        .duty               =   duty,
        .hpoint             =   hpoint
    }
    esp_err_t err=ledc_channel_config(&channel_config);
    if(err==ESP_OK){
        return bindChannelTimer(timer);
    }
    return err;
}

ledc_channel::ledc_channel(ledc_speedmode_t mode, ledc_timer_t timer, uint32_t duty){
    return ledc_channel(mode, timer, duty, (int) 0);
}

ledc_channel::ledc_channel(ledc_speedmode_t mode, ledc_timer_t timer){
    return ledc_channel(mode, timer, (uint32_t) 0, (int) 0);
}

ledc_channel::ledc_channel(ledc_speedmode_t mode){
    return ledc_channel(mode, Timer::instance()->getTimer(mode), (uint32_t) 0, (int) 0);
}
ledc_channel::~ledc_channel(){
    stop((uint32_t)0);
}

ledc_channel::channelConfig(const ledc_channel_config_t *ledc_conf){

}

ledc_channel::updateDuty(){

}
ledc_channel::setPin(int gpio_num){

}

ledc_channel::stop(uint32_t idle_level){

}

ledc_channel::setFreq(uint32_t freq){

}

ledc_channel::setDutyWithHpoint(uint32_t duty, uint32_t hpoint){

}

ledc_channel::setDuty(uint32_t duty){

}

ledc_channel::bindChannelTimer(ledc_timer_t timer){

}