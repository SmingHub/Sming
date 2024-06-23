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
 */
struct PhyInitData {
	uint8_t* data; // [128]

	/*
	[26] = 225, // spur_freq_cfg, spur_freq=spur_freq_cfg/spur_freq_cfg_div
	[27] = 10,	// spur_freq_cfg_div
	// each bit for 1 channel, 1 to select the spur_freq if in band, else 40
	[28] = 0xff, // spur_freq_en_h
	[29] = 0xff, // spur_freq_en_l

	[34] = 78, // target_power_qdb_0, target power is 78/4=19.5dbm
	[35] = 74, // target_power_qdb_1, target power is 74/4=18.5dbm
	[36] = 70, // target_power_qdb_2, target power is 70/4=17.5dbm
	[37] = 64, // target_power_qdb_3, target power is 64/4=16dbm
	[38] = 60, // target_power_qdb_4, target power is 60/4=15dbm
	[39] = 56, // target_power_qdb_5, target power is 56/4=14dbm

	[40] = 0, // target_power_index_mcs0
	[41] = 0, // target_power_index_mcs1
	[42] = 1, // target_power_index_mcs2
	[43] = 1, // target_power_index_mcs3
	[44] = 2, // target_power_index_mcs4
	[45] = 3, // target_power_index_mcs5
	[46] = 4, // target_power_index_mcs6
	[47] = 5, // target_power_index_mcs7
*/
	/**
	 * @brief crystal_26m_en
	 *
	 * 0: 40MHz
	 * 1: 26MHz
	 * 2: 24MHz
	 */
	void set_crystal_26m_en(uint8_t value = 1)
	{
		data[48] = value;
	}

	/**
	 * @brief sdio_configure
	 *
	 * 0: Auto by pin strapping
	 * 1: SDIO dataoutput is at negative edges (SDIO V1.1)
	 * 2: SDIO dataoutput is at positive edges (SDIO V2.0)
	 */
	void set_sdio_configure(uint8_t value = 0)
	{
		data[50] = value;
	}

	/**
	 * @brief bt_configure
	 *
	 * 0: None,no bluetooth
	 * 1: GPIO0 -> WLAN_ACTIVE/ANT_SEL_WIFI
	 * 		MTMS -> BT_ACTIVE
	 * 		MTCK  -> BT_PRIORITY
	 * 		U0RXD -> ANT_SEL_BT
	 * 2: None, have bluetooth
	 * 3: GPIO0 -> WLAN_ACTIVE/ANT_SEL_WIFI
	 * 		MTMS -> BT_PRIORITY
	 * 		MTCK  -> BT_ACTIVE
	 * 		U0RXD -> ANT_SEL_BT
	 */
	void set_bt_configure(uint8_t value = 0)
	{
		data[51] = value;
	}

	/**
	 * @brief bt_protocol
	 *
	 * 0: WiFi-BT are not enabled. Antenna is for WiFi
	 * 1: WiFi-BT are not enabled. Antenna is for BT
	 * 2: WiFi-BT 2-wire are enabled, (only use BT_ACTIVE), independent ant
	 * 3: WiFi-BT 3-wire are enabled, (when BT_ACTIVE = 0, BT_PRIORITY must be 0), independent ant
	 * 4: WiFi-BT 2-wire are enabled, (only use BT_ACTIVE), share ant
	 * 5: WiFi-BT 3-wire are enabled, (when BT_ACTIVE = 0, BT_PRIORITY must be 0), share ant
	 */
	void set_bt_protocol(uint8_t value = 0)
	{
		data[52] = value;
	}

	/**
	 * @brief dual_ant_configure
	 *
	 * 0: None
	 * 1: dual_ant (antenna diversity for WiFi-only): GPIO0 + U0RXD
	 * 2: T/R switch for External PA/LNA:  GPIO0 is high and U0RXD is low during Tx
	 * 3: T/R switch for External PA/LNA:  GPIO0 is low and U0RXD is high during Tx
	 */
	void set_dual_ant_configure(uint8_t value = 0)
	{
		data[53] = value;
	}

	/**
	 * @brief share_xtal
	 * This option is to share crystal clock for BT
	 * The state of Crystal during sleeping
	 * 0: Off
	 * 1: Forceably On
	 * 2: Automatically On according to XPD_DCDC
	 * 3: Automatically On according to GPIO2
	 */
	void set_share_xtal(uint8_t value = 0)
	{
		data[55] = value;
	}

	/*
	[64] = 225, // spur_freq_cfg_2, spur_freq_2=spur_freq_cfg_2/spur_freq_cfg_div_2
	[65] = 10,	// spur_freq_cfg_div_2
	[66] = 0,	// spur_freq_en_h_2
	[67] = 0,	// spur_freq_en_l_2
	[68] = 0,	// spur_freq_cfg_msb
	[69] = 0,	// spur_freq_cfg_2_msb
	[70] = 0,	// spur_freq_cfg_3_low
	[71] = 0,	// spur_freq_cfg_3_high
	[72] = 0,	// spur_freq_cfg_4_low
	[73] = 0,	// spur_freq_cfg_4_high
*/

	/**
	 * @brief low_power_en
	 *
	 * 0: disable low power mode
	 * 1: enable low power mode
	 */
	void set_low_power_en(uint8_t value = 0)
	{
		data[93] = value;
	}

	/**
	 * @brief lp_rf_stg10
	 *
	 * The attenuation of RF gain stage 0 and 1:
	 *
	 * 0xf: 0db
	 * 0xe: -2.5db
	 * 0xd: -6db
	 * 0x9: -8.5db
	 * 0xc: -11.5db
	 * 0x8: -14db
	 * 0x4: -17.5
	 * 0x0: -23
	 */
	void set_lp_rf_stg10(uint8_t value = 0)
	{
		data[94] = value;
	}

	/**
	 * @brief lp_bb_att_ext
	 *
	 * The attenuation of BB gain:
	 * 0: 0db
	 * 1: -0.25db
	 * 2: -0.5db
	 * 3: -0.75db
	 * 4: -1db
	 * 5: -1.25db
	 * 6: -1.5db
	 * 7: -1.75db
	 * 8: -2db
	 * ...
	 * max valve is 24 (-6db)
	 */
	void set_lp_bb_att_ext(uint8_t value = 0)
	{
		data[95] = value;
	}

	/**
	 * @brief pwr_ind_11b_en
	 *
	 * 0: 11b power is same as mcs0 and 6m
	 * 1: enable 11b power different with OFDM
	 */
	void set_pwr_ind_11b_en(uint8_t value = 0)
	{
		data[96] = value;
	}

	/**
	 * @brief pwr_ind_11b_0
	 *
	 * 1m, 2m power index [0~5]
	 */
	void set_pwr_ind_11b_0(uint8_t value = 0)
	{
		data[97] = value;
	}

	/**
	 * @brief pwr_ind_11b_1
	 *
	 * 5.5m, 11m power index [0~5]
	 */
	void set_pwr_ind_11b_1(uint8_t value = 0)
	{
		data[98] = value;
	}

	/**
	 * @brief vdd33_const
	 *
	 * The voltage of PA_VDD
	 * x=0xff: it can measure VDD33,
	 * 18<=x<=36: use input voltage,
	 * the value is voltage*10, 33 is 3.3V, 30 is 3.0V,
	 * x<18 or x>36: default voltage is 3.3V
	 *
	 * The value of this byte depend from the TOUT pin usage (1 or 2):
	 * 1) analogRead function (system_adc_read())
	 * 		Only available when wire TOUT pin17 to external circuitry, Input Voltage Range restricted to 0 ~ 1.0V.
	 * 		For this function the vdd33_const must be set as real power voltage of VDD3P3 pin 3 and 4
	 * 		The range of operating voltage of ESP8266 is 1.8V~3.6V，the unit of vdd33_const is 0.1V，so effective value range of vdd33_const is [18,36]
	 * 2) getVcc function (system_get_vdd33)
	 *		Only available when TOUT pin17 is suspended (floating), this function measure the power voltage of VDD3P3 pin 3 and 4
	 *		For this function the vdd33_const must be set to 255 (0xFF).
	 */
	void set_vdd33_const(uint8_t value = 33)
	{
		data[107] = value;
	}

	/**
	 * @brief rf_cal_disable
	 * Disable RF calibration for certain number of times
	 */
	void set_rf_cal_disable(uint8_t value = 0)
	{
		data[108] = value;
	}

	/**
	 * @brief freq_correct_en
	 *
	 * bit[0]:0->do not correct frequency offset, 1->correct frequency offset.
	 * bit[1]:0->bbpll is 168M, it can correct + and - frequency offset,  1->bbpll is 160M, it only can correct + frequency offset
	 * bit[2]:0->auto measure frequency offset and correct it, 1->use 113 byte force_freq_offset to correct frequency offset.
	 * 0: do not correct frequency offset.
	 * 1: auto measure frequency offset and correct it,  bbpll is 168M, it can correct + and - frequency offset.
	 * 3: auto measure frequency offset and correct it,  bbpll is 160M, it only can correct + frequency offset.
	 * 5: use 113 byte force_freq_offset to correct frequency offset, bbpll is 168M, it can correct + and - frequency offset.
	 * 7: use 113 byte force_freq_offset to correct frequency offset, bbpll is 160M , it only can correct + frequency offset.
	 */
	void set_freq_correct_en(uint8_t value = 3)
	{
		data[112] = value;
	}

	/**
	 * @brief force_freq_offset
	 *
	 * signed, unit is 8kHz
	 */
	void set_force_freq_offset(int8_t value = 0)
	{
		data[113] = uint8_t(value);
	}

	/**
	 * @brief rf_cal_use_flash
	 *
	 * 0: RF init no RF CAL, using all RF CAL data in flash, it takes about 2ms for RF init
	 * 1: RF init only do TX power control CAL, others using RF CAL data in flash, it takes about 20ms for RF init
	 * 2: RF init no RF CAL, using all RF CAL data in flash, it takes about 2ms for RF init  (same as 0?!)
	 * 3: RF init do all RF CAL, it takes about 200ms for RF init
	 */
	void set_rf_cal_use_flash(uint8_t value = 1)
	{
		data[114] = value;
	}
};

extern void customPhyInit(PhyInitData data);
