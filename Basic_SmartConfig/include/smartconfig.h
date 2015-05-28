/*
 * smartconfig.h
 *
 *  Created on: May 28, 2015
 *      Author: slavey
 */

#ifndef __SMARTCONFIG_H__
#define __SMARTCONFIG_H__

typedef enum {
    SC_STATUS_WAIT = 0,
    SC_STATUS_FIND_CHANNEL,
    SC_STATUS_GETTING_SSID_PSWD,
    SC_STATUS_LINK,
    SC_STATUS_LINK_OVER,
} sc_status;

typedef enum {
    SC_TYPE_ESPTOUCH = 0,
    SC_TYPE_AIRKISS,
} sc_type;

typedef void (*sc_callback_t)(sc_status status, void *pdata);

extern const char *smartconfig_get_version(void);
extern bool smartconfig_start(sc_type type, sc_callback_t cb, ...);
extern bool smartconfig_stop(void);


#endif /* SMARTCONFIG_H_ */
