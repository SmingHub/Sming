/* * Copyright (c)
 * 		2018 - present: Slavey Karadzhov <slav@attachix.com>
 * 		2014: Deoxxa Development (https://github.com/deoxxa/mqtt-protocol-c/)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *  * Neither the name of Slavey Karadzhov and Deoxxa Development nor the names
 * of its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MQTT_SERIALISER_H
#define MQTT_SERIALISER_H

#include "errors.h"
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mqtt_serialiser_rc_e {
  MQTT_SERIALISER_RC_ERROR,
  MQTT_SERIALISER_RC_SUCCESS,
} mqtt_serialiser_rc_t;

typedef struct mqtt_serialiser_s {
  mqtt_error_t error;
  size_t sequence;
} mqtt_serialiser_t;

void mqtt_serialiser_init(mqtt_serialiser_t* serialiser);
size_t mqtt_serialiser_size(mqtt_serialiser_t* serialiser, mqtt_message_t* message);
mqtt_serialiser_rc_t mqtt_serialiser_write(mqtt_serialiser_t* serialiser, mqtt_message_t* message, uint8_t* buffer,
                                           size_t len);
#ifdef __cplusplus
}
#endif

#endif
