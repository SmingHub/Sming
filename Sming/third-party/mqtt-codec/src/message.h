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

#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REVERSE_LINKED_LIST(TYPE, HEAD) \
  {                                     \
    TYPE* prev    = NULL;               \
    TYPE* current = *HEAD;              \
    TYPE* next;                         \
    while(current != NULL) {            \
      next          = current->next;    \
      current->next = prev;             \
      prev          = current;          \
      current       = next;             \
    }                                   \
    *HEAD = prev;                       \
  }

typedef enum mqtt_type_e {
  MQTT_TYPE_CONNECT     = 1,
  MQTT_TYPE_CONNACK     = 2,
  MQTT_TYPE_PUBLISH     = 3,
  MQTT_TYPE_PUBACK      = 4,
  MQTT_TYPE_PUBREC      = 5,
  MQTT_TYPE_PUBREL      = 6,
  MQTT_TYPE_PUBCOMP     = 7,
  MQTT_TYPE_SUBSCRIBE   = 8,
  MQTT_TYPE_SUBACK      = 9,
  MQTT_TYPE_UNSUBSCRIBE = 10,
  MQTT_TYPE_UNSUBACK    = 11,
  MQTT_TYPE_PINGREQ     = 12,
  MQTT_TYPE_PINGRESP    = 13,
  MQTT_TYPE_DISCONNECT  = 14,
} mqtt_type_t;

typedef enum mqtt_retain_e {
  MQTT_RETAIN_FALSE = 0,
  MQTT_RETAIN_TRUE  = 1,
} mqtt_retain_t;

typedef enum mqtt_qos_e {
  MQTT_QOS_AT_MOST_ONCE  = 0,
  MQTT_QOS_AT_LEAST_ONCE = 1,
  MQTT_QOS_EXACTLY_ONCE  = 2,
} mqtt_qos_t;

typedef enum mqtt_dup_e {
  MQTT_DUP_FALSE = 0,
  MQTT_DUP_TRUE  = 1,
} mqtt_dup_t;

typedef struct mqtt_topic_s {
  struct mqtt_topic_s* next;
  mqtt_buffer_t name;
} mqtt_topic_t;

typedef struct mqtt_topicpair_s {
  struct mqtt_topicpair_s* next;
  mqtt_buffer_t name;
  mqtt_qos_t qos;
} mqtt_topicpair_t;

#define MQTT_MESSAGE_COMMON_FIELDS \
  mqtt_retain_t retain;            \
  mqtt_qos_t qos;                  \
  mqtt_dup_t dup;                  \
  mqtt_type_t type;                \
  size_t length;

typedef union mqtt_message_u {
  struct {
    MQTT_MESSAGE_COMMON_FIELDS
  } common;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    mqtt_buffer_t protocol_name;
    uint8_t protocol_version;

    // Client to server
    struct {
      char username_follows;
      char password_follows;
      char will_retain;
      char will_qos;
      char will;
      char clean_session;
    } flags;

    uint16_t keep_alive;

    mqtt_buffer_t client_id;

    mqtt_buffer_t will_topic;
    mqtt_buffer_t will_message;

    mqtt_buffer_t username;
    mqtt_buffer_t password;
  } connect;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
    mqtt_topicpair_t* topics;
  } subscribe;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
    mqtt_topic_t* topics;
  } unsubscribe;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS
  } pingreq;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS
  } disconnect;

  // Server to client
  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint8_t _unused;
    uint8_t return_code;
  } connack;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
    mqtt_topicpair_t* topics;
  } suback;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
  } unsuback;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS
  } pingresp;

  // Bi-directional

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    mqtt_buffer_t topic_name;
    uint16_t message_id;

    mqtt_buffer_t content;
  } publish;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
  } puback;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
  } pubrec;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
  } pubrel;

  struct {
    MQTT_MESSAGE_COMMON_FIELDS

    uint16_t message_id;
  } pubcomp;

} mqtt_message_t;

void mqtt_message_init(mqtt_message_t* message);
void mqtt_message_clear(mqtt_message_t* message, int completely);

#ifdef DEBUG
void mqtt_message_dump(mqtt_message_t* message);
#endif

#ifdef __cplusplus
}
#endif

#endif
