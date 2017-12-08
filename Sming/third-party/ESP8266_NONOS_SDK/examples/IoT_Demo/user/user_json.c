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

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"

#include "user_json.h"

LOCAL char *json_buf;
LOCAL int pos;
LOCAL int size;

/******************************************************************************
 * FunctionName : find_json_path
 * Description  : find the JSON format tree's path
 * Parameters   : json -- A pointer to a JSON set up
 *                path -- A pointer to the JSON format tree's path
 * Returns      : A pointer to the JSON format tree
*******************************************************************************/
struct jsontree_value *ICACHE_FLASH_ATTR
find_json_path(struct jsontree_context *json, const char *path)
{
    struct jsontree_value *v;
    const char *start;
    const char *end;
    int len;

    v = json->values[0];
    start = path;

    do {
        end = (const char *)os_strstr(start, "/");

        if (end == start) {
            break;
        }

        if (end != NULL) {
            len = end - start;
            end++;
        } else {
            len = os_strlen(start);
        }

        if (v->type != JSON_TYPE_OBJECT) {
            v = NULL;
        } else {
            struct jsontree_object *o;
            int i;

            o = (struct jsontree_object *)v;
            v = NULL;

            for (i = 0; i < o->count; i++) {
                if (os_strncmp(start, o->pairs[i].name, len) == 0) {
                    v = o->pairs[i].value;
                    json->index[json->depth] = i;
                    json->depth++;
                    json->values[json->depth] = v;
                    json->index[json->depth] = 0;
                    break;
                }
            }
        }

        start = end;
    } while (end != NULL && *end != '\0' && v != NULL);

    json->callback_state = 0;
    return v;
}

/******************************************************************************
 * FunctionName : json_putchar
 * Description  : write the value to the JSON  format tree
 * Parameters   : c -- the value which write the JSON format tree
 * Returns      : result
*******************************************************************************/
int ICACHE_FLASH_ATTR
json_putchar(int c)
{
    if (json_buf != NULL && pos <= size) {
        json_buf[pos++] = c;
        return c;
    }

    return 0;
}

/******************************************************************************
 * FunctionName : json_ws_send
 * Description  : set up the JSON format tree for string
 * Parameters   : tree -- A pointer to the JSON format tree
 *                path -- A pointer to the JSON format tree's path
 *                pbuf -- A pointer for the data sent
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
json_ws_send(struct jsontree_value *tree, const char *path, char *pbuf)
{
    struct jsontree_context json;
    /* maxsize = 128 bytes */
    json_buf = (char *)os_malloc(jsonSize);

    /* reset state and set max-size */
    /* NOTE: packet will be truncated at 512 bytes */
    pos = 0;
    size = jsonSize;

    json.values[0] = (struct jsontree_value *)tree;
    jsontree_reset(&json);
    find_json_path(&json, path);
    json.path = json.depth;
    json.putchar = json_putchar;

    while (jsontree_print_next(&json) && json.path <= json.depth);

    json_buf[pos] = 0;
    os_memcpy(pbuf, json_buf, pos);
    os_free(json_buf);
}

/******************************************************************************
 * FunctionName : json_parse
 * Description  : parse the data as a JSON format
 * Parameters   : js_ctx -- A pointer to a JSON set up
 *                ptrJSONMessage -- A pointer to the data
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
json_parse(struct jsontree_context *json, char *ptrJSONMessage)
{
    /* Set value */
    struct jsontree_value *v;
    struct jsontree_callback *c;
    struct jsontree_callback *c_bak = NULL;

    while ((v = jsontree_find_next(json, JSON_TYPE_CALLBACK)) != NULL) {
        c = (struct jsontree_callback *)v;

        if (c == c_bak) {
            continue;
        }

        c_bak = c;

        if (c->set != NULL) {
            struct jsonparse_state js;

            jsonparse_setup(&js, ptrJSONMessage, os_strlen(ptrJSONMessage));
            c->set(json, &js);
        }
    }
}
