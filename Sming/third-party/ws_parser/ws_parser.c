#ifdef WS_PARSER_DUMP_STATE
    #include <stdio.h>
#endif

#include "ws_parser.h"

enum {
    S_OPCODE = 0,
    S_LENGTH,
    S_LENGTH_16_0,
    S_LENGTH_16_1,
    S_LENGTH_64_0,
    S_LENGTH_64_1,
    S_LENGTH_64_2,
    S_LENGTH_64_3,
    S_LENGTH_64_4,
    S_LENGTH_64_5,
    S_LENGTH_64_6,
    S_LENGTH_64_7,
    S_MASK_0,
    S_MASK_1,
    S_MASK_2,
    S_MASK_3,
    S_PAYLOAD,
};

void
ws_parser_init(ws_parser_t* parser, const ws_parser_callbacks_t* callbacks)
{
    parser->user_data = NULL;
    parser->callbacks = callbacks;
    parser->state = S_OPCODE;
    parser->fragment = 0;
}

#define ADVANCE           { buff++; len--; }
#define ADVANCE_AND_BREAK { ADVANCE; break; }

int
ws_parser_execute(ws_parser_t* parser, /* mutates! */ char* buff, size_t len)
{
    while(len) {
        uint8_t cur_byte = *buff;

        #ifdef WS_PARSER_DUMP_STATE
            printf("cur_byte=%d bytes_remaining=%llu fragment=%d fin=%d "
                   "control=%d mask_flag=%d mask_pos=%d state=%d len=%zu\n",
                (int)cur_byte,
                parser->bytes_remaining,
                parser->fragment,
                parser->fin,
                parser->control,
                parser->mask_flag,
                parser->mask_pos,
                parser->state,
                len);
        #endif

        switch(parser->state) {
            case S_OPCODE: {
                uint8_t opcode = cur_byte & 0x0f;

                if(cur_byte & 0x70) {
                    // reserved bits
                    return WS_RESERVED_BITS_SET;
                }

                parser->fin = (cur_byte & 0x80) ? 1 : 0;

                if(opcode == 0) { // continuation
                    if(!parser->fragment) {
                        return WS_INVALID_CONTINUATION;
                    }

                    parser->control = 0;
                } else if(opcode & 0x8) { // control
                    if(opcode != WS_FRAME_PING && opcode != WS_FRAME_PONG && opcode != WS_FRAME_CLOSE) {
                        return WS_INVALID_OPCODE;
                    }

                    if(!parser->fin) {
                        return WS_FRAGMENTED_CONTROL;
                    }

                    parser->control = 1;

                    int rc = parser->callbacks->on_control_begin(parser->user_data, opcode);
                    if(rc) {
                        return rc;
                    }
                } else { // data
                    if(opcode != WS_FRAME_TEXT && opcode != WS_FRAME_BINARY) {
                        return WS_INVALID_OPCODE;
                    }

                    parser->control = 0;
                    parser->fragment = !parser->fin;

                    int rc = parser->callbacks->on_data_begin(parser->user_data, opcode);
                    if(rc) {
                        return rc;
                    }
                }

                parser->state = S_LENGTH;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH: {
                uint8_t length = cur_byte & 0x7f;

                parser->mask_flag = (cur_byte & 0x80) ? 1 : 0;
                parser->mask_pos = 0;

                if(parser->control) {
                    if(length > 125) {
                        return WS_CONTROL_TOO_LONG;
                    }

                    parser->bytes_remaining = length;
                    parser->state = parser->mask_flag ? S_MASK_0 : S_PAYLOAD;
                } else {
                    if(length < 126) {
                        parser->bytes_remaining = length;
                        parser->state = parser->mask_flag ? S_MASK_0 : S_PAYLOAD;
                    } else if(length == 126) {
                        parser->state = S_LENGTH_16_0;
                    } else {
                        parser->state = S_LENGTH_64_0;
                    }
                }

                ADVANCE;

                if(parser->state == S_PAYLOAD && parser->bytes_remaining == 0) {
                    goto end_of_payload;
                }

                break;
            }
            case S_LENGTH_16_0: {
                parser->bytes_remaining = (uint64_t)cur_byte << 8;
                parser->state = S_LENGTH_16_1;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_16_1: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 0;
                parser->state = parser->mask_flag ? S_MASK_0 : S_PAYLOAD;

                if(parser->bytes_remaining < 126) {
                    return WS_NON_CANONICAL_LENGTH;
                }

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_0: {
                parser->bytes_remaining = (uint64_t)cur_byte << 56;
                parser->state = S_LENGTH_64_1;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_1: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 48;
                parser->state = S_LENGTH_64_2;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_2: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 40;
                parser->state = S_LENGTH_64_3;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_3: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 32;
                parser->state = S_LENGTH_64_4;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_4: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 24;
                parser->state = S_LENGTH_64_5;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_5: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 16;
                parser->state = S_LENGTH_64_6;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_6: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 8;
                parser->state = S_LENGTH_64_7;

                ADVANCE_AND_BREAK;
            }
            case S_LENGTH_64_7: {
                parser->bytes_remaining |= (uint64_t)cur_byte << 0;
                parser->state = parser->mask_flag ? S_MASK_0 : S_PAYLOAD;

                if(parser->bytes_remaining < 65536) {
                    return WS_NON_CANONICAL_LENGTH;
                }

                ADVANCE_AND_BREAK;
            }
            case S_MASK_0: {
                parser->mask[0] = cur_byte;
                parser->state = S_MASK_1;

                ADVANCE_AND_BREAK;
            }
            case S_MASK_1: {
                parser->mask[1] = cur_byte;
                parser->state = S_MASK_2;

                ADVANCE_AND_BREAK;
            }
            case S_MASK_2: {
                parser->mask[2] = cur_byte;
                parser->state = S_MASK_3;

                ADVANCE_AND_BREAK;
            }
            case S_MASK_3: {
                parser->mask[3] = cur_byte;
                parser->state = S_PAYLOAD;

                ADVANCE;

                if(parser->bytes_remaining == 0) {
                    goto end_of_payload;
                }

                break;
            }
            case S_PAYLOAD: {
                size_t chunk_length = len;

                if(chunk_length > parser->bytes_remaining) {
                    chunk_length = parser->bytes_remaining;
                }

                if(parser->mask_flag) {
                    size_t i = 0;
                    for(i = 0; i < chunk_length; i++) {
                        buff[i] ^= parser->mask[parser->mask_pos++];
                    }
                }

                int rc;

                if(parser->control) {
                    rc = parser->callbacks->on_control_payload(parser->user_data, buff, chunk_length);
                } else {
                    rc = parser->callbacks->on_data_payload(parser->user_data, buff, chunk_length);
                }

                if(rc) {
                    return rc;
                }

                buff += chunk_length;
                len -= chunk_length;
                parser->bytes_remaining -= chunk_length;

                if(parser->bytes_remaining == 0) {
                    goto end_of_payload;
                }

                break;
            }
            end_of_payload: {
                if(parser->control || parser->fin) {
                    int rc;

                    if(parser->control) {
                        rc = parser->callbacks->on_control_end(parser->user_data);
                    } else {
                        rc = parser->callbacks->on_data_end(parser->user_data);
                    }

                    if(rc) {
                        return rc;
                    }
                }

                parser->state = S_OPCODE;

                break;
            }
        }
    }

    return WS_OK;
}

const char*
ws_parser_error(int rc)
{
    #define XX(name, code) if(rc == code) return #name;
    WS_PARSER_ERROR_CODES(XX)
    #undef XX

    return NULL;
}
