/* * Copyright (c)
 * 		2018 - present: Slavey Karadzhov <slav@attachix.com>
 * 		2014: Deoxxa Development (https://github.com/deoxxa/mqtt-protocol-c/)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *  * Neither the name of Slavey Karadzhov and Deoxxa Development nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

#include "message.h"

void mqtt_message_init(mqtt_message_t* message) {
  memset(message, 0, sizeof(mqtt_message_t));
}

#ifdef DEBUG
void mqtt_message_dump(mqtt_message_t* message) {
  printf("message\n");
  printf("  type:              %d\n", message->common.type);
  printf("  qos:               %d\n", message->common.qos);
  printf("  dup:               %s\n", message->common.dup ? "true" : "false");
  printf("  retain:            %s\n", message->common.retain ? "true" : "false");

  if(message->common.type == MQTT_TYPE_CONNECT) {
    printf("  protocol name:     ");
    mqtt_buffer_dump(&(message->connect.protocol_name));
    printf("\n");

    printf("  protocol version:  %d\n", message->connect.protocol_version);

    printf("  has username:      %s\n", message->connect.flags.username_follows ? "true" : "false");
    printf("  has password:      %s\n", message->connect.flags.password_follows ? "true" : "false");
    printf("  has will:          %s\n", message->connect.flags.will ? "true" : "false");
    printf("  will qos:          %d\n", message->connect.flags.will_qos);
    printf("  retains will:      %s\n", message->connect.flags.will_retain ? "true" : "false");
    printf("  clean session:     %s\n", message->connect.flags.clean_session ? "true" : "false");

    printf("  keep alive:        %d\n", message->connect.keep_alive);

    printf("  client id:         ");
    mqtt_buffer_dump(&(message->connect.client_id));
    printf("\n");

    printf("  will topic:        ");
    mqtt_buffer_dump(&(message->connect.will_topic));
    printf("\n");
    printf("  will message:      ");
    mqtt_buffer_dump(&(message->connect.will_message));
    printf("\n");

    printf("  username:          ");
    mqtt_buffer_dump(&(message->connect.username));
    printf("\n");
    printf("  password:          ");
    mqtt_buffer_dump(&(message->connect.password));
    printf("\n");
  }
}
#endif

void mqtt_message_clear(mqtt_message_t* message, int completely) {
  if(!message) {
    return;
  }

  switch(message->common.type) {
    // Client to server messages
    case MQTT_TYPE_CONNECT: {
      MQTT_FREE(message->connect.protocol_name.data);
      MQTT_FREE(message->connect.client_id.data);
      MQTT_FREE(message->connect.will_topic.data);
      MQTT_FREE(message->connect.will_message.data);
      MQTT_FREE(message->connect.username.data);
      MQTT_FREE(message->connect.password.data);

      message->connect.protocol_name.data = NULL;
      message->connect.client_id.data     = NULL;
      message->connect.will_topic.data    = NULL;
      message->connect.will_message.data  = NULL;
      message->connect.username.data      = NULL;
      message->connect.password.data      = NULL;

      break;
    }

    case MQTT_TYPE_SUBSCRIBE: {
      mqtt_topicpair_t* cur = message->subscribe.topics;
      while(cur) {
        MQTT_FREE(cur->name.data);
        cur->name.data         = NULL;
        mqtt_topicpair_t* last = cur;
        cur                    = cur->next;
        MQTT_FREE(last);
      }
      message->subscribe.topics = NULL;

      break;
    }

    case MQTT_TYPE_UNSUBSCRIBE: {
      mqtt_topic_t* cur = message->unsubscribe.topics;
      while(cur) {
        mqtt_topic_t* last = cur;
        cur                = cur->next;
        MQTT_FREE(last);
      }
      message->unsubscribe.topics = NULL;

      break;
    }

    // Server to client messages
    case MQTT_TYPE_CONNACK: {
      break;
    }

    case MQTT_TYPE_SUBACK: {
      mqtt_topicpair_t* cur = message->suback.topics;
      while(cur) {
        mqtt_topicpair_t* last = cur;
        cur                    = cur->next;
        MQTT_FREE(last);
      }
      message->suback.topics = NULL;

      break;
    }

    case MQTT_TYPE_UNSUBACK: {
      break;
    }

    // Bi-directional messages
    case MQTT_TYPE_PUBLISH: {
      MQTT_FREE(message->publish.topic_name.data);
      message->publish.topic_name.data = NULL;

      MQTT_FREE(message->publish.content.data);
      message->publish.content.data = NULL;

      break;
    }

    case MQTT_TYPE_PUBACK:
    case MQTT_TYPE_PUBREC:
    case MQTT_TYPE_PUBREL:
    case MQTT_TYPE_PUBCOMP:
    // Below are all the message types that must not have variable header and payload
    case MQTT_TYPE_PINGREQ:
    case MQTT_TYPE_DISCONNECT:
    case MQTT_TYPE_PINGRESP:
      break;

    default: {
      return; // Invalid message?
    }
  }

  if(completely) {
    MQTT_FREE(message);
  }
}
