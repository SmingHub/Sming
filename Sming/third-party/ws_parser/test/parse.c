#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ws_parser.h"

int
on_data_begin(void* user_data, ws_frame_type_t frame_type)
{
    (void)user_data;

    printf("data_begin: %s\n",
        frame_type == WS_FRAME_TEXT   ? "text" :
        frame_type == WS_FRAME_BINARY ? "binary" :
        "?");

    return WS_OK;
}

int
on_data_payload(void* user_data, const char* buff, size_t len)
{
    (void)user_data;

    printf("data_payload: '%.*s'\n", (int)len, buff);

    return WS_OK;
}

int
on_data_end()
{
    printf("data_end\n");

    return WS_OK;
}

int
on_control_begin(void* user_data, ws_frame_type_t frame_type)
{
    (void)user_data;

    printf("control_begin: %s\n",
        frame_type == WS_FRAME_PING  ? "ping" :
        frame_type == WS_FRAME_PONG  ? "pong" :
        frame_type == WS_FRAME_CLOSE ? "close" :
        "?");

    return WS_OK;
}

int
on_control_payload(void* user_data, const char* buff, size_t len)
{
    (void)user_data;

    printf("control_payload: '%.*s'\n", (int)len, buff);

    return WS_OK;
}

int
on_control_end()
{
    printf("control_end\n");

    return WS_OK;
}

int
main()
{
    ws_parser_callbacks_t callbacks = {
        .on_data_begin      = on_data_begin,
        .on_data_payload    = on_data_payload,
        .on_data_end        = on_data_end,
        .on_control_begin   = on_control_begin,
        .on_control_payload = on_control_payload,
        .on_control_end     = on_control_end,
    };

    ws_parser_t parser;
    ws_parser_init(&parser, &callbacks);

    while(1) {
        char buff[4096];
        ssize_t nbytes = read(0, buff, sizeof(buff));

        if(nbytes < 0) {
            if(errno == EINTR) {
                continue;
            }

            perror("read");
            return 1;
        }

        if(nbytes == 0) {
            break;
        }

        int rc = ws_parser_execute(&parser, buff, nbytes);

        if(rc != WS_OK) {
            printf("error: %d %s\n", rc, ws_parser_error(rc));
            return 0;
        }
    }
}
