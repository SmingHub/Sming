#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t system_adc_read(void);
void system_adc_read_fast(uint16_t* adc_addr, uint16_t adc_num, uint8_t adc_clk_div);
uint16_t system_get_vdd33(void);

#ifdef __cplusplus
}
#endif

