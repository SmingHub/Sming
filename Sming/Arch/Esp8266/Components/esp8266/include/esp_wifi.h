/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * The bulk of the SDK `user_interface.h` file
 */

#pragma once

#include "os_type.h"
#include "ipv4_addr.h"
#include "../sdk/include/queue.h"
#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NULL_MODE 0x00
#define STATION_MODE 0x01
#define SOFTAP_MODE 0x02
#define STATIONAP_MODE 0x03

typedef enum _auth_mode {
	AUTH_OPEN = 0,
	AUTH_WEP,
	AUTH_WPA_PSK,
	AUTH_WPA2_PSK,
	AUTH_WPA_WPA2_PSK,
	AUTH_MAX,
} AUTH_MODE;

typedef enum _cipher_type {
	CIPHER_NONE = 0,
	CIPHER_WEP40,
	CIPHER_WEP104,
	CIPHER_TKIP,
	CIPHER_CCMP,
	CIPHER_TKIP_CCMP,
	CIPHER_UNKNOWN,
} CIPHER_TYPE;

uint8_t wifi_get_opmode(void);
uint8_t wifi_get_opmode_default(void);
bool wifi_set_opmode(uint8_t opmode);
bool wifi_set_opmode_current(uint8_t opmode);
uint8_t wifi_get_broadcast_if(void);
bool wifi_set_broadcast_if(uint8_t interface);

struct bss_info {
	STAILQ_ENTRY(bss_info) next;

	uint8_t bssid[6];
	uint8_t ssid[32];
	uint8_t ssid_len;
	uint8_t channel;
	sint8 rssi;
	AUTH_MODE authmode;
	uint8_t is_hidden;
	sint16 freq_offset;
	sint16 freqcal_val;
	uint8_t* esp_mesh_ie;
	uint8_t simple_pair;
	CIPHER_TYPE pairwise_cipher;
	CIPHER_TYPE group_cipher;
	uint32_t phy_11b : 1;
	uint32_t phy_11g : 1;
	uint32_t phy_11n : 1;
	uint32_t wps : 1;
	uint32_t reserved : 28;
};

typedef struct _scaninfo {
	STAILQ_HEAD(, bss_info) * pbss;
	struct espconn* pespconn;
	uint8_t totalpage;
	uint8_t pagenum;
	uint8_t page_sn;
	uint8_t data_cnt;
} scaninfo;

typedef enum {
	OK = 0,
	FAIL,
	PENDING,
	BUSY,
	CANCEL,
} STATUS;

typedef void (*scan_done_cb_t)(void* arg, STATUS status);

typedef struct {
	int8 rssi;
	AUTH_MODE authmode;
} wifi_fast_scan_threshold_t;

struct station_config {
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t channel;
	uint8_t bssid_set; // Note: If bssid_set is 1, station will just connect to the router
					   // with both ssid[] and bssid[] matched. Please check about this.
	uint8_t bssid[6];
	wifi_fast_scan_threshold_t threshold;
	bool open_and_wep_mode_disable; // Can connect to open/wep router by default.
	bool all_channel_scan;
};

bool wifi_station_get_config(struct station_config* config);
bool wifi_station_get_config_default(struct station_config* config);
bool wifi_station_set_config(struct station_config* config);
bool wifi_station_set_config_current(struct station_config* config);

bool wifi_station_connect(void);
bool wifi_station_disconnect(void);

void wifi_enable_signaling_measurement(void);
void wifi_disable_signaling_measurement(void);

sint8 wifi_station_get_rssi(void);

typedef enum {
	WIFI_SCAN_TYPE_ACTIVE = 0, /**< active scan */
	WIFI_SCAN_TYPE_PASSIVE,	/**< passive scan */
} wifi_scan_type_t;

/** @brief Range of active scan times per channel */
typedef struct {
	uint32_t min; /**< minimum active scan time per channel, units: millisecond */
	uint32_t max; /**< maximum active scan time per channel, units: millisecond, values above 1500ms may
                                          cause station to disconnect from AP and are not recommended.  */
} wifi_active_scan_time_t;

/** @brief Aggregate of active & passive scan time per channel */
typedef union {
	wifi_active_scan_time_t active; /**< active scan time per channel, units: millisecond. */
	uint32_t passive;				/**< passive scan time per channel, units: millisecond, values above 1500ms may
                                          cause station to disconnect from AP and are not recommended. */
} wifi_scan_time_t;

struct scan_config {
	uint8_t* ssid;				// Note: ssid == NULL, don't filter ssid.
	uint8_t* bssid;				// Note: bssid == NULL, don't filter bssid.
	uint8_t channel;			// Note: channel == 0, scan all channels, otherwise scan set channel.
	uint8_t show_hidden;		// Note: show_hidden == 1, can get hidden ssid routers' info.
	wifi_scan_type_t scan_type; // scan type, active or passive
	wifi_scan_time_t scan_time; // scan time per channel
};

bool wifi_station_scan(struct scan_config* config, scan_done_cb_t cb);

uint8_t wifi_station_get_auto_connect(void);
bool wifi_station_set_auto_connect(uint8_t set);

bool wifi_station_set_reconnect_policy(bool set);

enum {
	STATION_IDLE = 0,
	STATION_CONNECTING,
	STATION_WRONG_PASSWORD,
	STATION_NO_AP_FOUND,
	STATION_CONNECT_FAIL,
	STATION_GOT_IP
};

enum dhcp_status { DHCP_STOPPED, DHCP_STARTED };

uint8_t wifi_station_get_connect_status(void);

uint8_t wifi_station_get_current_ap_id(void);
bool wifi_station_ap_change(uint8_t current_ap_id);
bool wifi_station_ap_number_set(uint8_t ap_number);
uint8_t wifi_station_get_ap_info(struct station_config config[]);

bool wifi_station_dhcpc_start(void);
bool wifi_station_dhcpc_stop(void);
enum dhcp_status wifi_station_dhcpc_status(void);
bool wifi_station_dhcpc_set_maxtry(uint8_t num);

char* wifi_station_get_hostname(void);
bool wifi_station_set_hostname(char* name);

int wifi_station_set_cert_key(uint8_t* client_cert, int client_cert_len, uint8_t* private_key, int private_key_len,
							  uint8_t* private_key_passwd, int private_key_passwd_len);
void wifi_station_clear_cert_key(void);
int wifi_station_set_username(uint8_t* username, int len);
void wifi_station_clear_username(void);

struct softap_config {
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t ssid_len;		  // Note: Recommend to set it according to your ssid
	uint8_t channel;		  // Note: support 1 ~ 13
	AUTH_MODE authmode;		  // Note: Don't support AUTH_WEP in softAP mode.
	uint8_t ssid_hidden;	  // Note: default 0
	uint8_t max_connection;   // Note: default 4, max 4
	uint16_t beacon_interval; // Note: support 100 ~ 60000 ms, default 100
};

bool wifi_softap_get_config(struct softap_config* config);
bool wifi_softap_get_config_default(struct softap_config* config);
bool wifi_softap_set_config(struct softap_config* config);
bool wifi_softap_set_config_current(struct softap_config* config);

struct station_info {
	STAILQ_ENTRY(station_info) next;

	uint8_t bssid[6];
	struct ipv4_addr ip;
};

struct dhcps_lease {
	bool enable;
	struct ipv4_addr start_ip;
	struct ipv4_addr end_ip;
};

enum dhcps_offer_option {
	OFFER_START = 0x00,
	OFFER_ROUTER = 0x01,
	OFFER_END,
};

uint8_t wifi_softap_get_station_num(void);
struct station_info* wifi_softap_get_station_info(void);
bool wifi_softap_set_station_info (uint8_t* mac, struct ipv4_addr*);
void wifi_softap_free_station_info(void);

bool wifi_softap_dhcps_start(void);
bool wifi_softap_dhcps_stop(void);

bool wifi_softap_set_dhcps_lease(struct dhcps_lease* please);
bool wifi_softap_get_dhcps_lease(struct dhcps_lease* please);
uint32_t wifi_softap_get_dhcps_lease_time(void);
bool wifi_softap_set_dhcps_lease_time(uint32_t minute);
bool wifi_softap_reset_dhcps_lease_time(void);

enum dhcp_status wifi_softap_dhcps_status(void);
bool wifi_softap_set_dhcps_offer_option(uint8_t level, void* optarg);

#define STATION_IF 0x00
#define SOFTAP_IF 0x01

bool wifi_get_ip_info(uint8_t if_index, struct ip_info* info);
bool wifi_set_ip_info(uint8_t if_index, struct ip_info* info);
bool wifi_get_macaddr(uint8_t if_index, uint8_t* macaddr);
bool wifi_set_macaddr(uint8_t if_index, uint8_t* macaddr);

uint8_t wifi_get_channel(void);
bool wifi_set_channel(uint8_t channel);

void wifi_status_led_install(uint8_t gpio_id, uint32_t gpio_name, uint8_t gpio_func);
void wifi_status_led_uninstall();

void wifi_promiscuous_enable(uint8_t promiscuous);

typedef void (*wifi_promiscuous_cb_t)(uint8_t* buf, uint16_t len);

void wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);

void wifi_promiscuous_set_mac(const uint8_t* address);

enum phy_mode { PHY_MODE_11B = 1, PHY_MODE_11G = 2, PHY_MODE_11N = 3 };

enum phy_mode wifi_get_phy_mode(void);
bool wifi_set_phy_mode(enum phy_mode mode);

enum sleep_type { NONE_SLEEP_T = 0, LIGHT_SLEEP_T, MODEM_SLEEP_T };

enum sleep_level { MIN_SLEEP_T, MAX_SLEEP_T };

bool wifi_set_sleep_type(enum sleep_type type);
enum sleep_type wifi_get_sleep_type(void);
bool wifi_set_sleep_level(enum sleep_level level);
enum sleep_level wifi_get_sleep_level(void);
bool wifi_set_listen_interval(uint8_t interval);
uint8_t wifi_get_listen_interval(void);

void wifi_fpm_open(void);
void wifi_fpm_close(void);
void wifi_fpm_do_wakeup(void);
sint8 wifi_fpm_do_sleep(uint32_t sleep_time_in_us);
void wifi_fpm_set_sleep_type(enum sleep_type type);
enum sleep_type wifi_fpm_get_sleep_type(void);

typedef void (*fpm_wakeup_cb)(void);
void wifi_fpm_set_wakeup_cb(fpm_wakeup_cb cb);

void wifi_fpm_auto_sleep_set_in_null_mode(uint8_t req);

enum {
	EVENT_STAMODE_CONNECTED = 0,
	EVENT_STAMODE_DISCONNECTED,
	EVENT_STAMODE_AUTHMODE_CHANGE,
	EVENT_STAMODE_GOT_IP,
	EVENT_STAMODE_DHCP_TIMEOUT,
	EVENT_SOFTAPMODE_STACONNECTED,
	EVENT_SOFTAPMODE_STADISCONNECTED,
	EVENT_SOFTAPMODE_PROBEREQRECVED,
	EVENT_OPMODE_CHANGED,
	EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP,
	EVENT_MAX
};

enum {
	REASON_UNSPECIFIED = 1,
	REASON_AUTH_EXPIRE = 2,
	REASON_AUTH_LEAVE = 3,
	REASON_ASSOC_EXPIRE = 4,
	REASON_ASSOC_TOOMANY = 5,
	REASON_NOT_AUTHED = 6,
	REASON_NOT_ASSOCED = 7,
	REASON_ASSOC_LEAVE = 8,
	REASON_ASSOC_NOT_AUTHED = 9,
	REASON_DISASSOC_PWRCAP_BAD = 10,	  /* 11h */
	REASON_DISASSOC_SUPCHAN_BAD = 11,	 /* 11h */
	REASON_IE_INVALID = 13,				  /* 11i */
	REASON_MIC_FAILURE = 14,			  /* 11i */
	REASON_4WAY_HANDSHAKE_TIMEOUT = 15,   /* 11i */
	REASON_GROUP_KEY_UPDATE_TIMEOUT = 16, /* 11i */
	REASON_IE_IN_4WAY_DIFFERS = 17,		  /* 11i */
	REASON_GROUP_CIPHER_INVALID = 18,	 /* 11i */
	REASON_PAIRWISE_CIPHER_INVALID = 19,  /* 11i */
	REASON_AKMP_INVALID = 20,			  /* 11i */
	REASON_UNSUPP_RSN_IE_VERSION = 21,	/* 11i */
	REASON_INVALID_RSN_IE_CAP = 22,		  /* 11i */
	REASON_802_1X_AUTH_FAILED = 23,		  /* 11i */
	REASON_CIPHER_SUITE_REJECTED = 24,	/* 11i */

	REASON_BEACON_TIMEOUT = 200,
	REASON_NO_AP_FOUND = 201,
	REASON_AUTH_FAIL = 202,
	REASON_ASSOC_FAIL = 203,
	REASON_HANDSHAKE_TIMEOUT = 204,
};

typedef struct {
	uint8_t ssid[32];
	uint8_t ssid_len;
	uint8_t bssid[6];
	uint8_t channel;
} Event_StaMode_Connected_t;

typedef struct {
	uint8_t ssid[32];
	uint8_t ssid_len;
	uint8_t bssid[6];
	uint8_t reason;
} Event_StaMode_Disconnected_t;

typedef struct {
	uint8_t old_mode;
	uint8_t new_mode;
} Event_StaMode_AuthMode_Change_t;

typedef struct {
	struct ipv4_addr ip;
	struct ipv4_addr mask;
	struct ipv4_addr gw;
} Event_StaMode_Got_IP_t;

typedef struct {
	uint8_t mac[6];
	uint8_t aid;
} Event_SoftAPMode_StaConnected_t;

typedef struct {
	uint8_t mac[6];
	struct ipv4_addr ip;
	uint8_t aid;
} Event_SoftAPMode_Distribute_Sta_IP_t;

typedef struct {
	uint8_t mac[6];
	uint8_t aid;
} Event_SoftAPMode_StaDisconnected_t;

typedef struct {
	int rssi;
	uint8_t mac[6];
} Event_SoftAPMode_ProbeReqRecved_t;

typedef struct {
	uint8_t old_opmode;
	uint8_t new_opmode;
} Event_OpMode_Change_t;

typedef union {
	Event_StaMode_Connected_t connected;
	Event_StaMode_Disconnected_t disconnected;
	Event_StaMode_AuthMode_Change_t auth_change;
	Event_StaMode_Got_IP_t got_ip;
	Event_SoftAPMode_StaConnected_t sta_connected;
	Event_SoftAPMode_Distribute_Sta_IP_t distribute_sta_ip;
	Event_SoftAPMode_StaDisconnected_t sta_disconnected;
	Event_SoftAPMode_ProbeReqRecved_t ap_probereqrecved;
	Event_OpMode_Change_t opmode_changed;
} Event_Info_u;

typedef struct _esp_event {
	uint32_t event;
	Event_Info_u event_info;
} System_Event_t;

typedef void (*wifi_event_handler_cb_t)(System_Event_t* event);

void wifi_set_event_handler_cb(wifi_event_handler_cb_t cb);

typedef enum wps_type {
	WPS_TYPE_DISABLE = 0,
	WPS_TYPE_PBC,
	WPS_TYPE_PIN,
	WPS_TYPE_DISPLAY,
	WPS_TYPE_MAX,
} WPS_TYPE_t;

enum wps_cb_status {
	WPS_CB_ST_SUCCESS = 0,
	WPS_CB_ST_FAILED,
	WPS_CB_ST_TIMEOUT,
	WPS_CB_ST_WEP,
};

bool wifi_wps_enable(WPS_TYPE_t wps_type);
bool wifi_wps_disable(void);
bool wifi_wps_start(void);

typedef void (*wps_st_cb_t)(int status);
bool wifi_set_wps_cb(wps_st_cb_t cb);

typedef void (*freedom_outside_cb_t)(uint8_t status);
int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
void wifi_unregister_send_pkt_freedom_cb(void);
int wifi_send_pkt_freedom(uint8_t* buf, int len, bool sys_seq);

int wifi_rfid_locp_recv_open(void);
void wifi_rfid_locp_recv_close(void);

typedef void (*rfid_locp_cb_t)(uint8_t* frm, int len, int rssi);
int wifi_register_rfid_locp_recv_cb(rfid_locp_cb_t cb);
void wifi_unregister_rfid_locp_recv_cb(void);

enum FIXED_RATE {
	PHY_RATE_48 = 0x8,
	PHY_RATE_24 = 0x9,
	PHY_RATE_12 = 0xA,
	PHY_RATE_6 = 0xB,
	PHY_RATE_54 = 0xC,
	PHY_RATE_36 = 0xD,
	PHY_RATE_18 = 0xE,
	PHY_RATE_9 = 0xF,
};

#define FIXED_RATE_MASK_NONE 0x00
#define FIXED_RATE_MASK_STA 0x01
#define FIXED_RATE_MASK_AP 0x02
#define FIXED_RATE_MASK_ALL 0x03

int wifi_set_user_fixed_rate(uint8_t enable_mask, uint8_t rate);
int wifi_get_user_fixed_rate(uint8_t* enable_mask, uint8_t* rate);

enum support_rate {
	RATE_11B5M = 0,
	RATE_11B11M = 1,
	RATE_11B1M = 2,
	RATE_11B2M = 3,
	RATE_11G6M = 4,
	RATE_11G12M = 5,
	RATE_11G24M = 6,
	RATE_11G48M = 7,
	RATE_11G54M = 8,
	RATE_11G9M = 9,
	RATE_11G18M = 10,
	RATE_11G36M = 11,
};

int wifi_set_user_sup_rate(uint8_t min, uint8_t max);

enum RATE_11B_ID {
	RATE_11B_B11M = 0,
	RATE_11B_B5M = 1,
	RATE_11B_B2M = 2,
	RATE_11B_B1M = 3,
};

enum RATE_11G_ID {
	RATE_11G_G54M = 0,
	RATE_11G_G48M = 1,
	RATE_11G_G36M = 2,
	RATE_11G_G24M = 3,
	RATE_11G_G18M = 4,
	RATE_11G_G12M = 5,
	RATE_11G_G9M = 6,
	RATE_11G_G6M = 7,
	RATE_11G_B5M = 8,
	RATE_11G_B2M = 9,
	RATE_11G_B1M = 10
};

enum RATE_11N_ID {
	RATE_11N_MCS7S = 0,
	RATE_11N_MCS7 = 1,
	RATE_11N_MCS6 = 2,
	RATE_11N_MCS5 = 3,
	RATE_11N_MCS4 = 4,
	RATE_11N_MCS3 = 5,
	RATE_11N_MCS2 = 6,
	RATE_11N_MCS1 = 7,
	RATE_11N_MCS0 = 8,
	RATE_11N_B5M = 9,
	RATE_11N_B2M = 10,
	RATE_11N_B1M = 11
};

#define RC_LIMIT_11B 0
#define RC_LIMIT_11G 1
#define RC_LIMIT_11N 2
#define RC_LIMIT_P2P_11G 3
#define RC_LIMIT_P2P_11N 4
#define RC_LIMIT_NUM 5

#define LIMIT_RATE_MASK_NONE 0x00
#define LIMIT_RATE_MASK_STA 0x01
#define LIMIT_RATE_MASK_AP 0x02
#define LIMIT_RATE_MASK_ALL 0x03

bool wifi_set_user_rate_limit(uint8_t mode, uint8_t ifidx, uint8_t max, uint8_t min);
uint8_t wifi_get_user_limit_rate_mask(void);
bool wifi_set_user_limit_rate_mask(uint8_t enable_mask);

enum {
	USER_IE_BEACON = 0,
	USER_IE_PROBE_REQ,
	USER_IE_PROBE_RESP,
	USER_IE_ASSOC_REQ,
	USER_IE_ASSOC_RESP,
	USER_IE_MAX,
};

typedef void (*user_ie_manufacturer_recv_cb_t)(uint8_t type, const uint8_t sa[6], const uint8_t m_oui[3], uint8_t* ie,
											   uint8_t ie_len, int rssi);

bool wifi_set_user_ie(bool enable, uint8_t* m_oui, uint8_t type, uint8_t* user_ie, uint8_t len);
int wifi_register_user_ie_manufacturer_recv_cb(user_ie_manufacturer_recv_cb_t cb);
void wifi_unregister_user_ie_manufacturer_recv_cb(void);

void wifi_enable_gpio_wakeup(uint32_t i, GPIO_INT_TYPE intr_status);
void wifi_disable_gpio_wakeup(void);

void uart_div_modify(uint8_t uart_no, uint32_t DivLatchValue);

typedef enum {
	WIFI_COUNTRY_POLICY_AUTO, /**< Country policy is auto, use the country info of AP to which the station is connected */
	WIFI_COUNTRY_POLICY_MANUAL, /**< Country policy is manual, always use the configured country info */
} WIFI_COUNTRY_POLICY;

typedef struct {
	char cc[3];		/**< country code string */
	uint8_t schan;  /**< start channel */
	uint8_t nchan;  /**< total channel number */
	uint8_t policy; /**< country policy */
} wifi_country_t;

/**
  * @brief     configure country info
  *
  * @attention 1. The default country is {.cc="CN", .schan=1, .nchan=13, policy=WIFI_COUNTRY_POLICY_AUTO}
  * @attention 2. When the country policy is WIFI_COUNTRY_POLICY_AUTO, use the country info of AP to which the station is
  *               connected. E.g. if the configured country info is {.cc="USA", .schan=1, .nchan=11}, the country info of
  *               the AP to which the station is connected is {.cc="JP", .schan=1, .nchan=14}, then our country info is 
  *               {.cc="JP", .schan=1, .nchan=14}. If the station disconnected from the AP, the country info back to
  *               {.cc="USA", .schan=1, .nchan=11} again.
  * @attention 3. When the country policy is WIFI_COUNTRY_POLICY_MANUAL, always use the configured country info.
  * @attention 4. When the country info is changed because of configuration or because the station connects to a different
  *               external AP, the country IE in probe response/beacon of the soft-AP is changed also.
  * @attention 5. The country configuration is not stored into flash
  *
  * @param     wifi_country_t *country: the configured country info
  *
  * @return  true : succeed
  * @return false : fail
  */
bool wifi_set_country(wifi_country_t* country);

/**
  * @brief     get the current country info
  *
  * @param     wifi_country_t *country: country info
  *
  * @return  true : succeed
  * @return false : fail
  */
bool wifi_get_country(wifi_country_t* country);

#ifdef __cplusplus
}
#endif
