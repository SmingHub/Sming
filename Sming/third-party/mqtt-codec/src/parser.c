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

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "platform.h"

#include "parser.h"

#define READ_BYTES(NUM)                        \
  {                                            \
    size_t stored    = (parser->flags & 0x03); \
    size_t available = (len - parser->nread);  \
    if(stored + available < NUM) {             \
      return MQTT_PARSER_RC_INCOMPLETE;        \
    }                                          \
                                               \
    for(int x = stored; x < NUM; x++) {        \
      parser->stored[x] = data[parser->nread]; \
      parser->nread += 1;                      \
    }                                          \
    parser->flags = (parser->flags & 0xfc);    \
  }

static int read_string(mqtt_parser_t* parser, const uint8_t* data, size_t len, mqtt_buffer_t* into) {
  if(!(parser->flags & MQTT_PARSER_STATE_READ_STRING)) {
    READ_BYTES(2);

    parser->string_length = parser->stored[0] * 256 + parser->stored[1];
    into->length          = parser->string_length;
    parser->flags |= MQTT_PARSER_STATE_READ_STRING;
  }

  if(parser->buffer_pending == 0) {
    parser->buffer_length = into->length;

    return MQTT_PARSER_RC_WANT_MEMORY;
  }

  if(parser->buffer != NULL) {
    into->data            = parser->buffer;
    parser->buffer        = NULL;
    parser->buffer_length = 0;
  }
  size_t available = len - parser->nread;
  size_t consumed  = available < parser->string_length ? available : parser->string_length;
  size_t offset    = into->length - parser->string_length;
  memcpy(&into->data[offset], &data[parser->nread], consumed);
  parser->string_length -= consumed;
  parser->nread += consumed;
  if(parser->string_length > 0) {
    return MQTT_PARSER_RC_INCOMPLETE;
  }

  parser->buffer_pending = 0;
  parser->flags &= (~(MQTT_PARSER_STATE_READ_STRING));

  return -1;
}

#define READ_STRING(into)                           \
  {                                                 \
    int rc = read_string(parser, data, len, &into); \
    if(rc > -1) {                                   \
      return rc;                                    \
    }                                               \
  }

void mqtt_parser_init(mqtt_parser_t* parser, mqtt_parser_callbacks_t* callbacks) {
  parser->state          = MQTT_PARSER_STATE_INITIAL;
  parser->nread          = 0;
  parser->needs          = 0;
  parser->buffer_pending = 0;
  parser->buffer         = NULL;
  parser->buffer_length  = 0;
  memset(parser->stored, 0, sizeof(parser->stored));
  parser->string_length = 0;
  parser->flags         = 0;
  parser->callbacks     = callbacks;
}

void mqtt_parser_buffer(mqtt_parser_t* parser, uint8_t* buffer, size_t buffer_length) {
  parser->buffer_pending = 1;
  parser->buffer         = buffer;
  parser->buffer_length  = buffer_length;
}

static mqtt_parser_rc_t mqtt_parser_process(mqtt_parser_t* parser, mqtt_message_t* message, uint8_t* data, size_t len) {
  do {
    switch(parser->state) {
      case MQTT_PARSER_STATE_INITIAL: {
        READ_BYTES(1);

        message->common.retain = (parser->stored[0] >> 0) & 0x01;
        message->common.qos    = (parser->stored[0] >> 1) & 0x03;
        message->common.dup    = (parser->stored[0] >> 3) & 0x01;
        message->common.type   = (parser->stored[0] >> 4) & 0x0f;

        parser->state          = MQTT_PARSER_STATE_REMAINING_LENGTH;
        message->common.length = 0;
        parser->needs          = 0;

        break;
      }

      case MQTT_PARSER_STATE_REMAINING_LENGTH: {
        READ_BYTES(1);

        size_t multiplier = 1 << (7 * parser->needs);
        message->common.length += (parser->stored[0] & 0x7f) * multiplier;
        parser->needs += 1;

        if(!(parser->stored[0] >= 0x80 && parser->needs < 4)) {
          parser->state = MQTT_PARSER_STATE_REMAINING_LENGTH_DONE;
        }

        break;
      }

      case MQTT_PARSER_STATE_REMAINING_LENGTH_DONE: {
        parser->needs = 0;
        if(parser->stored[0] >= 0x80) {
          parser->error = MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH;
          return MQTT_PARSER_RC_ERROR;
        }

        int rc = parser->callbacks->on_message_begin(parser->data, message);
        if(rc) {
          return rc;
        }

#define CASE_TYPE(A)                       \
  case MQTT_TYPE_##A: {                    \
    parser->state = MQTT_PARSER_STATE_##A; \
    break;                                 \
  }
        switch(message->common.type) {
#if MQTT_ENABLE_SERVER
          CASE_TYPE(CONNECT)
          CASE_TYPE(SUBSCRIBE)
          CASE_TYPE(UNSUBSCRIBE)
#endif /* MQTT_ENABLE_SERVER */

#if MQTT_ENABLE_CLIENT
          CASE_TYPE(CONNACK)
          CASE_TYPE(SUBACK)
          CASE_TYPE(UNSUBACK)
#endif /* MQTT_ENABLE_CLIENT */

          CASE_TYPE(PUBLISH)
          CASE_TYPE(PUBACK)
          CASE_TYPE(PUBREC)
          CASE_TYPE(PUBREL)
          CASE_TYPE(PUBCOMP)

// Below are all the message types that must not have variable header and payload
#if MQTT_ENABLE_SERVER
          case MQTT_TYPE_PINGREQ:
          case MQTT_TYPE_DISCONNECT:
#endif /* MQTT_ENABLE_SERVER */
#if MQTT_ENABLE_CLIENT
          case MQTT_TYPE_PINGRESP:
#endif /* MQTT_ENABLE_CLIENT */

          default: {
            if(!message->common.length) {
              goto DONE;
            }

            parser->error = MQTT_ERROR_PARSER_INVALID_MESSAGE_ID;
            return MQTT_PARSER_RC_ERROR;
          }
        } /* message->common.type */

        break;
      }

      case MQTT_PARSER_STATE_VARIABLE_HEADER: {
#if MQTT_ENABLE_SERVER
        if(message->common.type == MQTT_TYPE_CONNECT) {
          parser->state = MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME;
        }
#endif

        break;
      }

#if MQTT_ENABLE_SERVER
      case MQTT_PARSER_STATE_CONNECT: {
        parser->state = MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_PROTOCOL_NAME: {
        READ_STRING(message->connect.protocol_name);

        parser->state = MQTT_PARSER_STATE_CONNECT_PROTOCOL_VERSION;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_PROTOCOL_VERSION: {
        READ_BYTES(1);

        message->connect.protocol_version = parser->stored[0];

        parser->state = MQTT_PARSER_STATE_CONNECT_FLAGS;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_FLAGS: {
        READ_BYTES(1);

        message->connect.flags.username_follows = (parser->stored[0] >> 7) & 0x01;
        message->connect.flags.password_follows = (parser->stored[0] >> 6) & 0x01;
        message->connect.flags.will_retain      = (parser->stored[0] >> 5) & 0x01;
        message->connect.flags.will_qos         = (parser->stored[0] >> 4) & 0x02;
        message->connect.flags.will             = (parser->stored[0] >> 2) & 0x01;
        message->connect.flags.clean_session    = (parser->stored[0] >> 1) & 0x01;

        parser->state = MQTT_PARSER_STATE_CONNECT_KEEP_ALIVE;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_KEEP_ALIVE: {
        READ_BYTES(2);

        message->connect.keep_alive = (parser->stored[0] << 8) + parser->stored[1];

        parser->state = MQTT_PARSER_STATE_CONNECT_CLIENT_IDENTIFIER;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_CLIENT_IDENTIFIER: {
        READ_STRING(message->connect.client_id);

        parser->state = MQTT_PARSER_STATE_CONNECT_WILL_TOPIC;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_WILL_TOPIC: {
        if(message->connect.flags.will) {
          READ_STRING(message->connect.will_topic);
        }

        parser->state = MQTT_PARSER_STATE_CONNECT_WILL_MESSAGE;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_WILL_MESSAGE: {
        if(message->connect.flags.will) {
          READ_STRING(message->connect.will_message);
        }

        parser->state = MQTT_PARSER_STATE_CONNECT_USERNAME;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_USERNAME: {
        if(message->connect.flags.username_follows) {
          READ_STRING(message->connect.username);
        }

        parser->state = MQTT_PARSER_STATE_CONNECT_PASSWORD;

        break;
      }

      case MQTT_PARSER_STATE_CONNECT_PASSWORD: {
        if(message->connect.flags.password_follows) {
          READ_STRING(message->connect.password);
        }

        goto DONE;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE: {
        READ_BYTES(2);

        message->subscribe.message_id = (parser->stored[0] << 8) + parser->stored[1];

        // Use the subscribe.length
        parser->needs = message->common.length - 2;

        parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPICS;
        break;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE_TOPICS: {
        mqtt_topicpair_t* next = NULL;
        if(message->subscribe.topics) {
          next = message->subscribe.topics;
        }

        message->subscribe.topics = (mqtt_topicpair_t*)MQTT_MALLOC(sizeof(mqtt_topicpair_t));
        memset(message->subscribe.topics, 0, sizeof(mqtt_topicpair_t));
        message->subscribe.topics->next = next;

        parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_NAME;
        break;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_NAME: {
        READ_STRING(message->subscribe.topics->name);

        parser->needs -= message->subscribe.topics->name.length + 2;
        parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_QOS;
        break;
      }

      case MQTT_PARSER_STATE_SUBSCRIBE_TOPIC_QOS: {
        READ_BYTES(1);

        message->subscribe.topics->qos = parser->stored[0];
        parser->needs -= 1;

        if((!parser->needs)) {
          REVERSE_LINKED_LIST(mqtt_topicpair_t, &message->subscribe.topics)
          goto DONE;
        }

        parser->state = MQTT_PARSER_STATE_SUBSCRIBE_TOPICS;
        break;
      }

      case MQTT_PARSER_STATE_UNSUBSCRIBE: {
        READ_BYTES(2);

        message->unsubscribe.message_id = (parser->stored[0] << 8) + parser->stored[1];
        parser->needs                   = message->common.length - 2;

        parser->state = MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS;
        break;
      }

      case MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS: {
        mqtt_topic_t* next = NULL;
        if(message->unsubscribe.topics) {
          next = message->unsubscribe.topics;
        }

        message->unsubscribe.topics = (mqtt_topic_t*)MQTT_MALLOC(sizeof(mqtt_topic_t));
        memset(message->unsubscribe.topics, 0, sizeof(mqtt_topic_t));
        message->unsubscribe.topics->next = next;

        parser->state = MQTT_PARSER_STATE_UNSUBSCRIBE_TOPIC_NAME;
        break;
      }

      case MQTT_PARSER_STATE_UNSUBSCRIBE_TOPIC_NAME: {
        READ_STRING(message->unsubscribe.topics->name);

        parser->needs -= message->unsubscribe.topics->name.length + 2;

        if(!parser->needs) {
          REVERSE_LINKED_LIST(mqtt_topic_t, &message->unsubscribe.topics)
          goto DONE;
        }

        parser->state = MQTT_PARSER_STATE_UNSUBSCRIBE_TOPICS;

        break;
      }

#endif /* MQTT_ENABLE_SERVER */

#if MQTT_ENABLE_CLIENT
      case MQTT_PARSER_STATE_CONNACK: {
        READ_BYTES(2);

        message->connack._unused     = parser->stored[0];
        message->connack.return_code = parser->stored[1];

        goto DONE;
      }

      case MQTT_PARSER_STATE_SUBACK: {
        READ_BYTES(2);

        message->suback.message_id = (parser->stored[0] << 8) + parser->stored[1];
        parser->state              = MQTT_PARSER_STATE_SUBACK_QOS;
        parser->needs              = message->common.length - 2;

        break;

        case MQTT_PARSER_STATE_SUBACK_QOS:
          READ_BYTES(1);

          mqtt_topicpair_t* next = NULL;
          if(message->suback.topics) {
            next = message->suback.topics;
          }

          message->suback.topics = (mqtt_topicpair_t*)MQTT_MALLOC(sizeof(mqtt_topicpair_t));
          memset(message->suback.topics, 0, sizeof(mqtt_topicpair_t));
          message->suback.topics->next = next;

          message->suback.topics->qos = parser->stored[0];
          parser->needs -= 1;

          if(!parser->needs) {
            REVERSE_LINKED_LIST(mqtt_topicpair_t, &message->suback.topics);
            goto DONE;
          }

          break;
      }

      case MQTT_PARSER_STATE_UNSUBACK: {
        READ_BYTES(2);

        message->unsuback.message_id = (parser->stored[0] << 8) + parser->stored[1];

        goto DONE;
      }

#endif /* MQTT_ENABLE_CLIENT */

      case MQTT_PARSER_STATE_PUBLISH: {
        parser->state = MQTT_PARSER_STATE_PUBLISH_TOPIC_NAME;
        parser->needs = message->common.length;
        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_TOPIC_NAME: {
        READ_STRING(message->publish.topic_name);
        parser->needs -= message->publish.topic_name.length + 2;

        // The Packet Identifier field is only present in PUBLISH Packets where the QoS level is 1 or 2.
        if(message->common.qos) {
          parser->state = MQTT_PARSER_STATE_PUBLISH_MESSAGE_ID;
        } else {
          parser->state = MQTT_PARSER_STATE_PUBLISH_PAYLOAD;
        }

        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_MESSAGE_ID: {
        READ_BYTES(2);

        message->publish.message_id = (parser->stored[0] << 8) + parser->stored[1];
        parser->needs -= 2;
        parser->state = MQTT_PARSER_STATE_PUBLISH_PAYLOAD;

        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_PAYLOAD: {
        message->publish.content.length = parser->needs;
        message->publish.content.data   = NULL;

        int rc = parser->callbacks->on_data_begin(parser->data, message);
        if(rc) {
          return rc;
        }

        parser->state = MQTT_PARSER_STATE_PUBLISH_PAYLOAD_READ;

        break;
      }

      case MQTT_PARSER_STATE_PUBLISH_PAYLOAD_READ: {
        int available = len - parser->nread;
        if(available < 1) {
          return MQTT_PARSER_RC_INCOMPLETE;
        }

        size_t consume = available < parser->needs ? available : parser->needs;

        int rc =
            parser->callbacks->on_data_payload(parser->data, message, (const char*)(data + parser->nread), consume);
        parser->nread += consume;
        parser->needs -= consume;
        if(rc) {
          return rc;
        }

        if(!parser->needs) {
          rc = parser->callbacks->on_data_end(parser->data, message);
          if(rc) {
            return rc;
          }

          goto DONE;
        }

        break;
      }

      case MQTT_PARSER_STATE_PUBACK: {
        READ_BYTES(2);

        message->puback.message_id = (parser->stored[0] << 8) + parser->stored[1];

        goto DONE;
      }

      case MQTT_PARSER_STATE_PUBREC: {
        READ_BYTES(2);

        message->pubrec.message_id = (parser->stored[0] << 8) + parser->stored[1];

        goto DONE;
      }

      case MQTT_PARSER_STATE_PUBREL: {
        READ_BYTES(2);

        message->pubrel.message_id = (parser->stored[0] << 8) + parser->stored[1];

        goto DONE;
      }

      case MQTT_PARSER_STATE_PUBCOMP: {
        READ_BYTES(2);

        message->pubcomp.message_id = (parser->stored[0] << 8) + parser->stored[1];

        goto DONE;
      }

      DONE : {
        int rc = parser->callbacks->on_message_end(parser->data, message);
        mqtt_message_clear(message, 0);
        if(rc) {
          return rc;
        }

        parser->state = MQTT_PARSER_STATE_INITIAL;
        parser->needs = 0;

        break;
      }

      default: {
        parser->error = MQTT_ERROR_PARSER_INVALID_STATE;

        return MQTT_PARSER_RC_ERROR;
      }
    }
  } while(1);
}

mqtt_parser_rc_t mqtt_parser_execute(mqtt_parser_t* parser, mqtt_message_t* message, uint8_t* data, size_t len) {
  int rc = 0;

  parser->nread = 0;

  // main loop
  do {
    rc = mqtt_parser_process(parser, message, data, len);
    if(rc == MQTT_PARSER_RC_WANT_MEMORY) {
      mqtt_parser_buffer(parser, MQTT_MALLOC(parser->buffer_length), parser->buffer_length);
    }
  } while(rc == MQTT_PARSER_RC_CONTINUE || rc == MQTT_PARSER_RC_WANT_MEMORY);

  if(parser->nread != len) {
    MQTT_DEBUGF("Overflow bytes: %zu\n", (len - parser->nread));

    size_t overflow = (len - parser->nread);
    if(overflow > sizeof(parser->stored)) {
      MQTT_DEBUGF("ERROR: Overflow is too big: %zu\n", overflow);
      return MQTT_PARSER_RC_ERROR;
    }

    for(int x = 0; x < overflow; x++) {
      parser->stored[x] = data[parser->nread];
      parser->nread += 1;
    }

    parser->flags = (parser->flags & 0xfc) | overflow;
  }

  return rc;
}
