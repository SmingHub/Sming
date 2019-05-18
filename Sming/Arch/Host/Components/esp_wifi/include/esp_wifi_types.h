#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_systemapi.h>

#define STATION_IF 0x00
#define SOFTAP_IF 0x01

/* Platform/WifiEvents.h */

struct ip_info {
	struct ip_addr ip;
	struct ip_addr netmask;
	struct ip_addr gw;
};

/* System events */

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
	EVENT_MAX,
};

typedef struct {
	uint8 ssid[32];
	uint8 ssid_len;
	uint8 bssid[6];
	uint8 channel;
} Event_StaMode_Connected_t;

typedef struct {
	uint8 ssid[32];
	uint8 ssid_len;
	uint8 bssid[6];
	uint8 reason;
} Event_StaMode_Disconnected_t;

typedef struct {
	uint8 old_mode;
	uint8 new_mode;
} Event_StaMode_AuthMode_Change_t;

typedef struct {
	struct ip_addr ip;
	struct ip_addr mask;
	struct ip_addr gw;
} Event_StaMode_Got_IP_t;

typedef struct {
	uint8 mac[6];
	uint8 aid;
} Event_SoftAPMode_StaConnected_t;

typedef struct {
	uint8 mac[6];
	struct ip_addr ip;
	uint8 aid;
} Event_SoftAPMode_Distribute_Sta_IP_t;

typedef struct {
	uint8 mac[6];
	uint8 aid;
} Event_SoftAPMode_StaDisconnected_t;

typedef struct {
	int rssi;
	uint8 mac[6];
} Event_SoftAPMode_ProbeReqRecved_t;

typedef struct {
	uint8 old_opmode;
	uint8 new_opmode;
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
	uint32 event;
	Event_Info_u event_info;
} System_Event_t;

/* Platform/Station.h */

enum STATUS {
	OK,
	FAIL,
	PENDING,
	BUSY,
	CANCEL,
};

struct bss_info;

enum AUTH_MODE {
	AUTH_OPEN,
	AUTH_WEP,
	AUTH_WPA_PSK,
	AUTH_WPA2_PSK,
	AUTH_WPA_WPA2_PSK,
	AUTH_MAX,
};

/* Platform/AccessPoint */

struct softap_config;

#ifdef __cplusplus
}
#endif
