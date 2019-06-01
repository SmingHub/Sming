#pragma once

#include "esp_wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define NULL_MODE       0x00
//#define STATION_MODE    0x01
//#define SOFTAP_MODE     0x02
//#define STATIONAP_MODE  0x03

//uint8 wifi_get_opmode(void);
//uint8 wifi_get_opmode_default(void);
//bool wifi_set_opmode(uint8 opmode);
//bool wifi_set_opmode_current(uint8 opmode);
//uint8 wifi_get_broadcast_if(void);
//bool wifi_set_broadcast_if(uint8 interface);

//bool wifi_station_get_config(struct station_config *config);
//bool wifi_station_get_config_default(struct station_config *config);
//bool wifi_station_set_config(struct station_config *config);
//bool wifi_station_set_config_current(struct station_config *config);

//bool wifi_station_connect(void);
//bool wifi_station_disconnect(void);

//void wifi_enable_signaling_measurement(void);
//void wifi_disable_signaling_measurement(void);

//sint8 wifi_station_get_rssi(void);

//uint8 wifi_station_get_connect_status(void);

//uint8 wifi_station_get_current_ap_id(void);
//bool wifi_station_ap_change(uint8 current_ap_id);
//bool wifi_station_ap_number_set(uint8 ap_number);
//uint8 wifi_station_get_ap_info(struct station_config config[]);

//bool wifi_station_dhcpc_start(void);
//bool wifi_station_dhcpc_stop(void);
//enum dhcp_status wifi_station_dhcpc_status(void);
//bool wifi_station_dhcpc_set_maxtry(uint8 num);

//char* wifi_station_get_hostname(void);
//bool wifi_station_set_hostname(char *name);

//int wifi_station_set_cert_key(uint8 *client_cert, int client_cert_len,
//    uint8 *private_key, int private_key_len,
//    uint8 *private_key_passwd, int private_key_passwd_len);
//void wifi_station_clear_cert_key(void);
//int wifi_station_set_username(uint8 *username, int len);
//void wifi_station_clear_username(void);

//bool wifi_softap_get_config(struct softap_config *config);
//bool wifi_softap_get_config_default(struct softap_config *config);
//bool wifi_softap_set_config(struct softap_config *config);
//bool wifi_softap_set_config_current(struct softap_config *config);

//uint8 wifi_softap_get_station_num(void);
//struct station_info * wifi_softap_get_station_info(void);
//void wifi_softap_free_station_info(void);

//bool wifi_softap_dhcps_start(void);
//bool wifi_softap_dhcps_stop(void);

//bool wifi_softap_set_dhcps_lease(struct dhcps_lease *please);
//bool wifi_softap_get_dhcps_lease(struct dhcps_lease *please);
//uint32 wifi_softap_get_dhcps_lease_time(void);
//bool wifi_softap_set_dhcps_lease_time(uint32 minute);
//bool wifi_softap_reset_dhcps_lease_time(void);

//enum dhcp_status wifi_softap_dhcps_status(void);
//bool wifi_softap_set_dhcps_offer_option(uint8 level, void* optarg);

bool wifi_get_ip_info(uint8 if_index, struct ip_info *info);
//bool wifi_set_ip_info(uint8 if_index, struct ip_info *info);
bool wifi_get_macaddr(uint8 if_index, uint8 *macaddr);
//bool wifi_set_macaddr(uint8 if_index, uint8 *macaddr);

//uint8 wifi_get_channel(void);
//bool wifi_set_channel(uint8 channel);

//void wifi_status_led_install(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func);
//void wifi_status_led_uninstall();

//void wifi_promiscuous_enable(uint8 promiscuous);

//typedef void (* wifi_promiscuous_cb_t)(uint8 *buf, uint16 len);

//void wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);

//void wifi_promiscuous_set_mac(const uint8_t *address);

//enum phy_mode wifi_get_phy_mode(void);
//bool wifi_set_phy_mode(enum phy_mode mode);

typedef void (* wifi_event_handler_cb_t)(System_Event_t *event);

void wifi_set_event_handler_cb(wifi_event_handler_cb_t cb);

//bool wifi_wps_enable(WPS_TYPE_t wps_type);
//bool wifi_wps_disable(void);
//bool wifi_wps_start(void);

//typedef void (*wps_st_cb_t)(int status);
//bool wifi_set_wps_cb(wps_st_cb_t cb);

//typedef void (*freedom_outside_cb_t)(uint8 status);
//int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
//void wifi_unregister_send_pkt_freedom_cb(void);
//int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);

//int wifi_rfid_locp_recv_open(void);
//void wifi_rfid_locp_recv_close(void);

//typedef void (*rfid_locp_cb_t)(uint8 *frm, int len, int rssi);
//int wifi_register_rfid_locp_recv_cb(rfid_locp_cb_t cb);
//void wifi_unregister_rfid_locp_recv_cb(void);

//int wifi_set_user_fixed_rate(uint8 enable_mask, uint8 rate);
//int wifi_get_user_fixed_rate(uint8 *enable_mask, uint8 *rate);

//int wifi_set_user_sup_rate(uint8 min, uint8 max);

//bool wifi_set_user_rate_limit(uint8 mode, uint8 ifidx, uint8 max, uint8 min);
//uint8 wifi_get_user_limit_rate_mask(void);
//bool wifi_set_user_limit_rate_mask(uint8 enable_mask);

//typedef void (*user_ie_manufacturer_recv_cb_t)(uint8 type, const uint8 sa[6], const uint8 m_oui[3], uint8 *ie, uint8 ie_len, int rssi);

//bool wifi_set_user_ie(bool enable, uint8 *m_oui, uint8 type, uint8 *user_ie, uint8 len);
//int wifi_register_user_ie_manufacturer_recv_cb(user_ie_manufacturer_recv_cb_t cb);
//void wifi_unregister_user_ie_manufacturer_recv_cb(void);

//bool wifi_set_country(wifi_country_t *country);
//bool wifi_get_country(wifi_country_t *country);

void host_wifi_lwip_init_complete(void);


#ifdef __cplusplus
}
#endif
