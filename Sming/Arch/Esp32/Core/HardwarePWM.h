/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwarePWM.h
 *
 ****/

 #pragma once

 #include <IHardwarePWM.h>
 
 /**
  * @brief ESP32-specific implementation of pulse width modulation
  * 
  * ESP8266 has no hardware PWM and uses an interrupt-based software implementation.
  * Period of PWM is fixed to 1000us / Frequency = 1kHz by default
  * For a period of 1000, max duty = 25000
  * You can use function setPeriod() to change frequency/period.
  */




  /* *  @param  usePhaseShift Use ledc's hpoint phase shift feature to stagger the rising edge of the PWM 
	 * 						  singals evenly across the period. This is useful for driving multiple LEDs
	 * 						  connected to the same power rail to reduce the peak current draw and help with 
	 * 						  electromagnetic interference.
	 *  @param  useSpreadSpectrum implement minimal spread spectrum modulation to further reduce EMI. This 
	 * 						  feature uses a hardware timer to modulate the base frequency of the PWM signal.
	 * 						  Depending on the base frequency, this may cause quite some system load, so 
	 *                        choosing a good balance between CPU resources and acceptable EMI is important.
     * 
     * */
 class HardwarePWM : public IHardwarePWM
 {
 public:
     /** @brief  Instantiate hardware PWM object
      *  @param  pins Pointer to array of pins to control
      *  @param  no_of_pins Quantity of elements in array of pins
      *  @param  usePhaseShift Parameter ignored on ESP8266 (present for API compatibility)
      */
     HardwarePWM(const uint8_t* pins, uint8_t no_of_pins);
     HardwarePWM(const uint8_t* pins, uint8_t no_of_pins, PWM_Options& options);
    
     virtual ~HardwarePWM();
 
      /** @brief  initialize PWM hardware
      *  @param  pins Pointer to array of pins to control
      *  @param  no_of_pins Quantity of elements in array of pins
      *  @param  options PWM options
      */
     void Init(const uint8_t* pins, uint8_t no_of_pins, PWM_Options& options);

     /** @brief  Set PWM duty cycle for a channel
      *  @param  chan Channel to set
      *  @param  duty Value of duty cycle to set channel to
      *  @param  update Update PWM output
      *  @retval bool True on success
      */
     bool setDutyChan(uint8_t chan, uint32_t duty, bool update = true) override;
 
     /** @brief  Get PWM duty cycle
      *  @param  chan Channel to get duty cycle for
      *  @retval uint32_t Value of PWM duty cycle
      */
     uint32_t getDutyChan(uint8_t chan) const override;
 
     /** @brief  Set PWM period
      *  @param  period PWM period in microseconds
      */
     void setPeriod(uint32_t period) override;
 
     /** @brief  Get PWM period
      *  @retval uint32_t Value of PWM period in microseconds
      */
     uint32_t getPeriod() const override;
 
     /** @brief  Get channel number for a pin
      *  @param  pin GPIO to interrogate
      *  @retval uint8_t Channel of GPIO
      */
     uint8_t getChannel(uint8_t pin) const override;
 
     /** @brief  Get the maximum duty cycle value
      *  @retval uint32_t Maximum permissible duty cycle
      */
     uint32_t getMaxDuty() const override;
 
     /** @brief  Update PWM to use new settings
      */
     void update() override;
 
     /** @brief Get PWM Frequency
      *  @param pin GPIO to get frequency for
      *  @retval uint32_t Value of Frequency 
      */
     uint32_t getFrequency(uint8_t pin) const override;
 
     
 private:
     int hpointForPin(uint8_t channelIndex, uint8_t channel_count);

     uint8_t channel_count;
     uint8_t channels[PWM_CHANNEL_NUM_MAX];
     uint32_t maxduty{0};
     PWM_Options _options;
 };