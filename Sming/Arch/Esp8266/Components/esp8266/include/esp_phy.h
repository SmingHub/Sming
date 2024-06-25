/*
 Adapted from Arduino for Sming.
 Original copyright note is kept below.

 phy.c - ESP8266 PHY initialization data
 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <esp_attr.h>
#include <sys/pgmspace.h>
#include <string.h>

/**
 * @brief Structure to manage low-level adjustment of PHY data
 * Does not contain the data but a reference to it.
 * The data should not be accessed directly.
 * @see For further details, see the following PDF documents:
 * 	- ESP8266 Phy Init Bin Parameter Configuration Guide
 * 	- ESP8266 Non-OS SDK API Reference (Final version is version 4)
 */
struct PhyInitData {
	/**
	 * @brief Index for configurable power level, from 0-5. See `PhyInitData::set_txpwr_dqb`.
	 */
	using txpwr_index_t = uint8_t;

	uint8_t* data; // [128]

	/**
	 * @brief PHY data version number. Currently version 8.
	 */
	uint8_t get_version() const
	{
		return data[1];
	}

	/**
	 * @brief Ordinal 26-29
	 *
	 * These values are poorly defined.
	 *
	 * - [26] = 225,  // spur_freq_cfg, spur_freq=spur_freq_cfg/spur_freq_cfg_div
	 * - [27] = 10,	  // spur_freq_cfg_div
	 *
	 * Each bit for 1 channel, 1 to select the spur_freq if in band, else 40
	 *
	 * - [28] = 0xff, // spur_freq_en_h
	 * - [29] = 0xff, // spur_freq_en_l
	 */
	void undocumented_26_29()
	{
	}

	/**
	 * @brief Configure the maximum TX powers for channels 1, 11, 13 and 14.
	 * @param chan1 Limit for channel 1
	 * @param chan11 Limit for channel 11
	 * @param chan13 Limit for channel 13
	 * @param chan14 Limit for channel 14
	 */
	void set_power_limits(txpwr_index_t chan1, txpwr_index_t chan11, txpwr_index_t chan13, txpwr_index_t chan14)
	{
		data[78] = 2; // Enable bytes 30-33 to set maximum TX power
		data[30] = chan1;
		data[31] = chan11;
		data[32] = chan13;
		data[33] = chan14;
	}

	/**
	 * @brief Disable power limits on channels 1, 11, 13 and 14
	 */
	void disable_power_limits()
	{
		data[78] = 0; // disable bytes 30-33
	}

	/**
	 * @brief Set TX power level.
	 * TX power can be switched between six levels.
	 * Level 0 represents the maximum TX power, level 5 the minimum.
	 * @param level Index from 0-5
	 * @param value Power level in 0.25dB increments
	 * @note Defaults are:
	 * - 0: 78/4 = 19.5dBm
	 * - 1: 74/4 = 18.5dBm
	 * - 2: 70/4 = 17.5dBm
	 * - 3: 64/4 = 16dBm
	 * - 4: 60/4 = 15dBm
	 * - 5: 56/4 = 14dBm
	 *
	 * @see See ESP8266 Non-OS SDK API Reference:
	 * 	- system_phy_set_max_tpw
	 */
	void set_txpwr_dqb(txpwr_index_t level, uint8_t value)
	{
		if(level < 6) {
			data[34 + level] = value;
		}
	}

	/**
	 * @brief Select target power level for specific data rate according to Modulation Coding Scheme (MCS)
	 * @param mcs_index Modulation Coding Scheme (MCS) index, from 0-7
	 * @param txpwr_qdb Power level 0-5. See `PhyInitData::set_txpwr_dqb`.
	 *
	 * @note The defaults are:
	 * - MCS0: qdb_0	1 Mbit/s, 2 Mbit/s, 5.5 Mbit/s, 11 Mbit/s, 6 Mbit/s, 9 Mbit/s)
	 * - MCS1: qdb_0	12 Mbit/s)
	 * - MCS2: qdb_1	18 Mbit/s)
	 * - MCS3: qdb_1	24 Mbit/s
	 * - MCS4: qdb_2	36 Mbit/s
	 * - MCS5: qdb_3	48 Mbit/s
	 * - MCS6: qdb_4	54 Mbit/s
	 * - MCS7: qdb_5
	 */
	void set_txpwr(uint8_t mcs_index, txpwr_index_t txpwr_qdb)
	{
		if(mcs_index < 8 && txpwr_qdb < 6) {
			data[40 + mcs_index] = txpwr_qdb;
		}
	}

	/**
	 * @brief Select crystal frequency
	 * @param value Values are:
	 * - 0: 40MHz
	 * - 1: 26MHz
	 * - 2: 24MHz
	 */
	void set_crystal_freq(uint8_t value = 1)
	{
		data[48] = value;
	}

	/**
	 * @brief Configure SDIO behaviour
	 * @param value Values are:
	 * - 0: Auto by pin strapping
	 * - 1: SDIO data output is at negative edges (SDIO V1.1)
	 * - 2: SDIO data output is at positive edges (SDIO V2.0)
	 */
	void set_sdio_configure(uint8_t value = 0)
	{
		data[50] = value;
	}

	/**
	 * @brief Configure bluetooth
	 * @param value Values are:
	 * - 0: None,no bluetooth
	 * - 1: Enable, pins are:
	 * 		- GPIO0 -> WLAN_ACTIVE/ANT_SEL_WIFI
	 * 		- MTMS -> BT_ACTIVE
	 * 		- MTCK  -> BT_PRIORITY
	 * 		- U0RXD -> ANT_SEL_BT
	 * - 2: None, have bluetooth
	 * - 3: Enable, pins are:
	 * 		- GPIO0 -> WLAN_ACTIVE/ANT_SEL_WIFI
	 * 		- MTMS -> BT_PRIORITY
	 * 		- MTCK  -> BT_ACTIVE
	 * 		- U0RXD -> ANT_SEL_BT
	 */
	void set_bt_configure(uint8_t value = 0)
	{
		data[51] = value;
	}

	/**
	 * @brief Configure Bluetooth protocol
	 * @param value Values are:
	 * - 0: WiFi-BT are not enabled. Antenna is for WiFi
	 * - 1: WiFi-BT are not enabled. Antenna is for BT
	 * - 2: WiFi-BT 2-wire are enabled, (only use BT_ACTIVE), independent ant
	 * - 3: WiFi-BT 3-wire are enabled, (when BT_ACTIVE = 0, BT_PRIORITY must be 0), independent ant
	 * - 4: WiFi-BT 2-wire are enabled, (only use BT_ACTIVE), share ant
	 * - 5: WiFi-BT 3-wire are enabled, (when BT_ACTIVE = 0, BT_PRIORITY must be 0), share ant
	 */
	void set_bt_protocol(uint8_t value = 0)
	{
		data[52] = value;
	}

	/**
	 * @brief Configure dual antenna arrangement
	 * @param value Values are:
	 * - 0: None
	 * - 1: dual_ant (antenna diversity for WiFi-only): GPIO0 + U0RXD
	 * - 2: T/R switch for External PA/LNA:  GPIO0 is high and U0RXD is low during Tx
	 * - 3: T/R switch for External PA/LNA:  GPIO0 is low and U0RXD is high during Tx
	 */
	void set_dual_ant_configure(uint8_t value = 0)
	{
		data[53] = value;
	}

	/**
	 * @brief Set Crystal state during sleep mode
	 * 	This option is to share crystal clock for BT
	 * @param value The state of Crystal during sleeping:
	 * 	- 0: Off
	 * 	- 1: Forceably On
	 * 	- 2: Automatically On according to XPD_DCDC
	 * 	- 3: Automatically On according to GPIO2
	 */
	void set_share_xtal(uint8_t value = 0)
	{
		data[55] = value;
	}

	/**
	 * @brief Ordinal 64-73
	 *
	 * These values are poorly defined.
	 *
	 * - [64] = 225, // spur_freq_cfg_2, spur_freq_2=spur_freq_cfg_2/spur_freq_cfg_div_2
	 * - [65] = 10,	 // spur_freq_cfg_div_2
	 * - [66] = 0,	 // spur_freq_en_h_2
	 * - [67] = 0,	 // spur_freq_en_l_2
	 * - [68] = 0,	 // spur_freq_cfg_msb
	 * - [69] = 0,	 // spur_freq_cfg_2_msb
	 * - [70] = 0,	 // spur_freq_cfg_3_low
	 * - [71] = 0,	 // spur_freq_cfg_3_high
	 * - [72] = 0,	 // spur_freq_cfg_4_low
	 * - [73] = 0,	 // spur_freq_cfg_4_high
	*/
	void undocumented_64_73()
	{
	}

	/**
	 * @brief Configure low-power mode
	 * @param value Values are:
	 * 	- 0: disable low power mode
	 * 	- 1: enable low power mode
	 */
	void set_low_power_en(uint8_t value = 0)
	{
		data[93] = value;
	}

	/**
	 * @brief Set attenuation of RF gain stage 0 and 1
	 * @param value Values are:
	 * - 0x0f: 0dB
	 * - 0x0e: -2.5dB
	 * - 0x0d: -6dB
	 * - 0x09: -8.5dB
	 * - 0x0c: -11.5dB
	 * - 0x08: -14dB
	 * - 0x04: -17.5dB
	 * - 0x00: -23dB
	 */
	void set_lp_rf_stg10(uint8_t value = 0)
	{
		data[94] = value;
	}

	/**
	 * @brief Set attenuation of BB gain
	 * @param value Attentuation in 0.25dB steps. Max valve is 24 (-6dB):
	 * - 0: 0dB
	 * - 1: -0.25dB
	 * - 2: -0.5dB
	 * - 3: -0.75dB
	 * - 4: -1dB
	 * - 5: -1.25dB
	 * - 6: -1.5dB
	 * - 7: -1.75dB
	 * - 8: -2dB
	 * etc.
	 */
	void set_lp_bb_att_ext(uint8_t value = 0)
	{
		data[95] = value;
	}

	/**
	 * @brief Set power limits for 802.11b to default,
	 * which is same as MCS0 and 6Mbits/s modes. See `PhyInitData::set_txpwr`.
	 */
	void set_default_power_limits_11b()
	{
		data[96] = 0;
	}

	/**
	 * @brief Configure 802.11b power limits
	 * @param txpwr_index_11b_0 Power level for 1Mbit/s and 2Mbit/s modes
	 * @param txpwr_index_11b_1 Power level for 5.5Mbits/s and 11Mbits/s modes
	 */
	void set_power_limits_11b(txpwr_index_t txpwr_index_11b_0, txpwr_index_t txpwr_index_11b_1)
	{
		data[96] = 1;
		data[97] = txpwr_index_11b_0;
		data[98] = txpwr_index_11b_1;
	}

	/**
	 * @brief Set PA_VDD voltage
	 * @param value Values are:
	 * - 0xff: Can measure VDD33
	 * - 18 <= value <= 36: use input voltage, where `value = voltage * 10`, so 33 is 3.3V, 30 is 3.0V, etc.
	 * - value < 18, value > 36: default voltage is 3.3V
	 *
	 * The value of this byte depends on the TOUT pin usage:
	 * - analogRead function: `system_adc_read()`
	 *		- Only available when wire TOUT pin 17 to external circuitry, Input Voltage Range restricted to 0 ~ 1.0V.
	 * 		- For this function the vdd33_const must be set as real power voltage of VDD3P3 pin 3 and 4
	 * 		- The range of operating voltage of ESP8266 is 1.8V ~ 3.6V，the unit of vdd33_const is 0.1V, so effective value range of vdd33_const is [18, 36].
	 * - getVcc function: `system_get_vdd33()`
	 *		- Only available when TOUT pin 17 is suspended (floating), this function measure the power voltage of VDD3P3 pin 3 and 4
	 *		- For this function the vdd33_const must be set to 255 (0xFF).
	 *
	 * @see See ESP8266 Non-OS SDK API Reference:
	 * - system_get_vdd33
	 * - system_adc_read
	 * - system_adc_read_fast
	 */
	void set_vdd33_const(uint8_t value = 33)
	{
		data[107] = value;
	}

	/**
	 * @brief Disable RF calibration for certain number of times
	 * @param value Number of times to disable RF calibration
	 * @see See ESP8266 Non-OS SDK API Reference:
	 * - system_deep_sleep_set_option
	 * - system_phy_set_rfoption
	 */
	void set_rf_cal_disable(uint8_t value = 0)
	{
		data[108] = value;
	}

	/**
	 * @brief Enable frequency correction
	 * @param mode Correction mode:
	 * - 0x00: do not correct frequency offset
	 * - 0x01: auto-correct, bbpll 168M, can correct positive and negative offsets
	 * - 0x03: auto-correct, bbpll 160M, can only correct positive offsets
	 * - 0x05: Correct using `force_freq_offset`, bbpll 168M, offset can be any value
	 * - 0x07: Correct using `force_freq_offset`, bbpll 160M, offset must >= 0
	 * @param force_freq_offset Correction figure in 8kHz steps, signed
	 *
	 * Bits are defined as follows:
	 * - bit 0
	 * 		- 0: do not correct frequency offset
	 * 		- 1: correct frequency offset
	 * - bit 1
	 * 		- 0: bbpll is 168M, it can correct + and - frequency offset
	 * 		- 1: bbpll is 160M, it only can correct + frequency offset
	 * - bit 2
	 * 		- 0: auto measure frequency offset and correct it
	 * 		- 1: use force_freq_offset to correct frequency offset
	 */
	void set_freq_correct(uint8_t bbpll = 3, int8_t force_freq_offset = 0)
	{
		data[112] = bbpll;
		data[113] = uint8_t(force_freq_offset);
	}

	/**
	 * @brief RF_calibration
	 * @param value Values are:
	 * - 0: RF init no RF CAL, using all RF CAL data in flash, it takes about 2ms for RF init
	 * - 1: RF init only do TX power control CAL, others using RF CAL data in flash, it takes about 20ms for RF init
	 * - 2: RF init no RF CAL, using all RF CAL data in flash, it takes about 2ms for RF init  (same as 0?!)
	 * - 3: RF init do all RF CAL, it takes about 200ms for RF init
	 * @note To ensure better RF performance, it is recommend to set RF_calibration to 3, otherwise the RF performance may become poor.
	 * @see See ESP8266 Non-OS SDK API Reference:
	 * 	- system_phy_set_powerup_option
	 */
	void set_rf_calibration(uint8_t value = 1)
	{
		data[114] = value;
	}
};

extern void customPhyInit(PhyInitData data);
