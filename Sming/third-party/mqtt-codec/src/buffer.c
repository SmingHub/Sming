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

#include <stdio.h>

#include "platform.h"

#include "buffer.h"

#ifdef DEBUG
void mqtt_buffer_dump(mqtt_buffer_t* buffer) {
  printf("[%d] ", buffer->length);

  char hex = 0;
  for(size_t i = 0; i < buffer->length; ++i) {
    if(buffer->data[i] < 0x20 || buffer->data[i] > 0x7e) {
      hex = 1;
      break;
    }
  }

  if(hex) {
    mqtt_buffer_dump_hex(buffer);
  } else {
    mqtt_buffer_dump_ascii(buffer);
  }
}

void mqtt_buffer_dump_ascii(mqtt_buffer_t* buffer) {
  for(size_t i = 0; i < buffer->length; ++i) {
    printf("%c", buffer->data[i]);
  }
}

void mqtt_buffer_dump_hex(mqtt_buffer_t* buffer) {
  for(size_t i = 0; i < buffer->length; ++i) {
    printf("%02x ", buffer->data[i]);
  }
}
#endif
