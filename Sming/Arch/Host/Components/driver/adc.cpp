#include <driver/adc.h>
#include <string.h>
#include <esp_system.h>

uint16_t system_adc_read(void)
{
	os_delay_us(20);
	return 0;
}

void system_adc_read_fast(uint16_t* adc_addr, uint16_t adc_num, uint8_t adc_clk_div)
{
	os_delay_us(6 * adc_num);
	memset(adc_addr, 0, adc_num * sizeof(uint16_t));
}

uint16_t system_get_vdd33(void)
{
	os_delay_us(20);
	return 0;
}
