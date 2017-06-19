# ESP8266_new_pwm
This is a drop-in replacement for the ESP8266 SDK PWM

The software PWM provided in the ESP8266 SDK from Espressif has several drawbacks:

1. Duty cycle limited to 90% (at 1kHz PWM period)
2. usable PWM period at most ~2KHz.
3. Incomplete documentation
 
This replacement allows duty cycles from 0% to 100%, with a stepsize of 200ns.
This is 5000 steps for a 1kHz PWM, and 256 steps (8 bit of resolution) at 19kHz.

If all channels are in steady state (either 0% of 100% in any combination),
the implementation goes to full idle, e.g. no interrupts.

The code is a drop-in replacement for the SDK, it provides the same functions
as the SDK libpwm.a closed binary library. Just add pwm.c to your project.

By default there is one small difference to the SDK. The code uses a unit of
200ns for both period and duty. E.g. for 10% duty cycle at 1kHz you need to
specify a period value of 5000 and a duty cycle value of 500, a duty cycle of
5000 or above switches the channel to full on.

To have full compatibility with the SDK, you have to set the
SDK_PWM_PERIOD_COMPAT_MODE define to 1. If set, the code will use 1us for PWM
period and 40ns for the duty cycle. E.g. 10% duty cycle at 1kHz is set by a
period value of 1000 and a duty cycle value of 2500, full duty at 25000 and
above.

Example usage:

	#define PWM_CHANNELS 5
	const uint32_t period = 5000 // * 200ns ^= 1 kHz

	// PWM setup
	uint32 io_info[PWM_CHANNELS][3] = {
		// MUX, FUNC, PIN
		{PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12, 12},
		{PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15, 15},
		{PERIPHS_IO_MUX_MTCK_U,  FUNC_GPIO13, 13},
		{PERIPHS_IO_MUX_MTMS_U,  FUNC_GPIO14, 14},
		{PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5 ,  5},
	};

	// initial duty: all off
	uint32 pwm_duty_init[PWM_CHANNELS] = {0, 0, 0, 0, 0};

	pwm_init(period, pwm_duty_init, PWM_CHANNELS, io_info);
	pwm_start();

	// do something like this whenever you want to change duty
	pwm_set_duty(500, 1);  // GPIO15: 10%
	pwm_set_duty(5000, 1); // GPIO15: 100%
	pwm_start();           // commit

**CAVEATS**

- **To set 100% duty, the duty must be *equal* to the period**
- **The code uses the TIMER1 interrupt. If you use e.g. the
  softtimer, there is a conflict. You can use NM1 for the PWM
  instead.**

