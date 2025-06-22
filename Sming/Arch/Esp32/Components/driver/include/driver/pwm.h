/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * pwm.h
 *
 ****/

#pragma once

#include <soc/soc_caps.h>

#define PWM_CHANNEL_NUM_MAX SOC_LEDC_CHANNEL_NUM

/**
 * @file pwm.h
 * @brief PWM (Pulse Width Modulation) configuration structures and enumerations for ESP32.
 */

/**
 * @enum PWM_phase_shift_mode
 * @brief Defines the modes for phase shifting in PWM.
 * 
 * @var PWM_PHASE_OFF
 * No phase shifting.
 * 
 * @var PWM_PHASE_CUSTOM
 * Use custom per-channel phase delay values.
 * 
 * @var PWM_PHASE_AUTO
 * Evenly distribute phases across the PWM period.
 */
enum PWM_phase_shift_mode {
    PWM_PHASE_OFF,           // No phase shifting
    PWM_PHASE_CUSTOM,        // Use custom per-channel values
    PWM_PHASE_AUTO    // Evenly distribute phases across period
};

/**
 * @enum PWM_spread_spectrum_mode
 * @brief Defines the modes for spread spectrum in PWM.
 * 
 * @var PWM_SPREAD_OFF
 * No spread spectrum.
 * 
 * @var PWM_SPREAD_CUSTOM
 * Use custom per-channel frequency offset values.
 * 
 * @var PWM_SPREAD_AUTO
 * Automatically apply spread spectrum with a specified percentage.
 */
enum PWM_spread_spectrum_mode {
    PWM_SPREAD_OFF,
    PWM_SPREAD_CUSTOM,
    PWM_SPREAD_AUTO
};

/**
 * @struct PWM_phase_shift
 * @brief Configuration structure for PWM phase shifting.
 * 
 * @var PWM_phase_shift::mode
 * Specifies the phase shift mode. See @ref PWM_phase_shift_mode.
 * 
 * @var PWM_phase_shift::phaseDelayPercent
 * Array of phase delay percentages for each channel (used when mode is PWM_PHASE_CUSTOM).
 * 
 * @var PWM_phase_shift::phaseStartPercent
 * Optional starting phase percentage for automatic phase distribution (used when mode is PWM_PHASE_AUTO).
 */
 struct PWM_phase_shift {
    PWM_phase_shift_mode mode = PWM_PHASE_OFF;
    union {
        uint8_t phaseDelayPercent[PWM_CHANNEL_NUM_MAX]; // Used when mode is PWM_PHASE_CUSTOM
        uint8_t phaseStartPercent;                      // Optional starting phase for PWM_PHASE_AUTO
    } ;
};

/**
 * @struct PWM_spread_spectrum
 * @brief Configuration structure for PWM spread spectrum.
 * 
 * @var PWM_spread_spectrum::mode
 * Specifies the spread spectrum mode. See @ref PWM_spread_spectrum_mode.
 * 
 * @var PWM_spread_spectrum::frequencyOffsetHz
 * Array of frequency offsets in Hz for each channel (used when mode is PWM_SPREAD_CUSTOM).
 * 
 * @var PWM_spread_spectrum::spreadPercentage
 * Spread percentage for automatic spread spectrum (used when mode is PWM_SPREAD_AUTO).
 */
struct PWM_spread_spectrum {
    PWM_spread_spectrum_mode mode = PWM_SPREAD_OFF;
    union {
        int32_t frequencyOffsetHz[PWM_CHANNEL_NUM_MAX];     // Used when mode is PWM_SPREAD_CUSTOM
        uint8_t spreadPercentage;                         // Percentage for PWM_SPREAD_AUTO (0.0-100.0)
    } ;
};

/**
 * @struct PWM_Options
 * @brief Configuration options for PWM (Pulse Width Modulation).
 *
 * This structure defines the configuration parameters for PWM, including
 * phase shift and spread spectrum options.
 *
 * @var PWM_Options::phaseShift
 * Specifies the phase shift configuration for the PWM signal.
 * 
 * @var PWM_Options::spreadSpectrum
 * Specifies the spread spectrum configuration for the PWM signal.
 */
struct PWM_Options {
    PWM_phase_shift phaseShift;
    PWM_spread_spectrum spreadSpectrum;
};