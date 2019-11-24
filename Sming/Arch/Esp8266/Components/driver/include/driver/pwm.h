#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <pwm.h>

/**
 * @defgroup pwm_driver PWM driver
 * @ingroup drivers
 * @{
 */

/**
 * @fn void pwm_init(uint32 period, uint32 *duty,uint32 pwm_channel_num,uint32 (*pin_info_list)[3])
 * @brief Initialize PWM function, including GPIO selection, period and duty cycle
 * @param period PWM period
 * @param duty duty cycle of each output
 * @param pwm_channel_num PWM channel number
 * @param pin_info_list Array containing an entry for each channel giving
 * @note This API can be called only once.
 *
 * Example:
 *
 * 		uint32 io_info[][3] = {
 * 			{PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC, PWM_0_OUT_IO_NUM},
 * 			{PWM_1_OUT_IO_MUX, PWM_1_OUT_IO_FUNC, PWM_1_OUT_IO_NUM},
 * 			{PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC, PWM_2_OUT_IO_NUM}
 *		};
 *
 *		pwm_init(light_param.pwm_period, light_param.pwm_duty, 3, io_info);
 *
 */

/**
 * @fn void pwm_start(void)
 * @brief Starts PWM
 *
 * This function needs to be called after PWM configuration is changed.
 */

/**
 * @fn void pwm_set_duty(uint32 duty, uint8 channel)
 * @brief Sets duty cycle of a PWM output
 * @param duty The time that high-level single will last, duty cycle will be (duty*45)/(period*1000)
 * @param channel PWM channel, which depends on how many PWM channels are used
 *
 * Set the time that high-level signal will last.
 * The range of duty depends on PWM period. Its maximum value of which can be Period * 1000 / 45.
 *
 * For example, for 1-KHz PWM, the duty range is 0 ~ 22222.
 */

/**
 * @fn uint32 pwm_get_duty(uint8 channel)
 * @brief Get duty cycle of PWM output
 * @param channel PWM channel, which depends on how many PWM channels are used
 * @retval uint32 Duty cycle of PWM output
 *
 * Duty cycle will be (duty*45) / (period*1000).
 */

/**
 * @fn void pwm_set_period(uint32 period)
 * @brief Set PWM period
 * @param period PWM period in us. For example, 1-KHz PWM period = 1000us.
 */

/**
 * @fn uint32 pwm_get_period(void)
 * @brief Get PWM period
 * @retval uint32 Return PWM period in us.
 */

/**
 * @fn uint32 get_pwm_version(void)
 * @brief Get version information of PWM
 * @retval uint32 PWM version
 */

/** @} */

#if defined(__cplusplus)
}
#endif
