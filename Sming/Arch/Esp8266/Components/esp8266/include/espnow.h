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

#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum esp_now_role {
	ESP_NOW_ROLE_IDLE = 0,
	ESP_NOW_ROLE_CONTROLLER,
	ESP_NOW_ROLE_SLAVE,
	ESP_NOW_ROLE_COMBO,
	ESP_NOW_ROLE_MAX,
};

typedef void (*esp_now_recv_cb_t)(uint8_t* mac_addr, uint8_t* data, uint8_t len);
typedef void (*esp_now_send_cb_t)(uint8_t* mac_addr, uint8_t status);

int esp_now_init(void);
int esp_now_deinit(void);

int esp_now_register_send_cb(esp_now_send_cb_t cb);
int esp_now_unregister_send_cb(void);

int esp_now_register_recv_cb(esp_now_recv_cb_t cb);
int esp_now_unregister_recv_cb(void);

int esp_now_send(uint8_t* da, uint8_t* data, int len);

int esp_now_add_peer(uint8_t* mac_addr, uint8_t role, uint8_t channel, uint8_t* key, uint8_t key_len);
int esp_now_del_peer(uint8_t* mac_addr);

int esp_now_set_self_role(uint8_t role);
int esp_now_get_self_role(void);

int esp_now_set_peer_role(uint8_t* mac_addr, uint8_t role);
int esp_now_get_peer_role(uint8_t* mac_addr);

int esp_now_set_peer_channel(uint8_t* mac_addr, uint8_t channel);
int esp_now_get_peer_channel(uint8_t* mac_addr);

int esp_now_set_peer_key(uint8_t* mac_addr, uint8_t* key, uint8_t key_len);
int esp_now_get_peer_key(uint8_t* mac_addr, uint8_t* key, uint8_t* key_len);

uint8_t* esp_now_fetch_peer(bool restart);

int esp_now_is_peer_exist(uint8_t* mac_addr);

int esp_now_get_cnt_info(uint8_t* all_cnt, uint8_t* encrypt_cnt);

int esp_now_set_kok(uint8_t* key, uint8_t len);

#ifdef __cplusplus
}
#endif
