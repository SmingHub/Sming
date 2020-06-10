/**
 * LWIP adapter compatibility
 */
#pragma once

/**
  A big thank you to Hristo Gochkov and his Asynchronous TCP library
  Big chunks of the code are inspired from this project.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <Platform/DefaultLwipAdapter.h>

#if 0

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "espinc/lwip_includes.h"

#ifndef DEBUG_TCP_EXTENDED
#define DEBUG_TCP_EXTENDED 1
#endif

// Settings
tcp_pcb* async_tcp_new();
void async_tcp_arg(struct tcp_pcb* pcb, void* arg);

// Actions
err_t async_tcp_connect(struct tcp_pcb* pcb, const ip_addr_t* ipaddr, u16_t port, tcp_connected_fn callback);
tcp_pcb* async_tcp_listen_with_backlog(struct tcp_pcb* pcb, u8_t backlog);
void async_tcp_accept(struct tcp_pcb* pcb, tcp_accept_fn callback);
err_t async_tcp_bind(struct tcp_pcb* pcb, const ip_addr_t* ipaddr, u16_t port);
err_t async_tcp_write(struct tcp_pcb* pcb, const void* dataptr, u16_t len, u8_t apiflags);
void async_tcp_recved(struct tcp_pcb* pcb, u16_t len);
err_t async_tcp_output(struct tcp_pcb* pcb);
void async_tcp_abort(struct tcp_pcb* pcb);
err_t async_tcp_close(struct tcp_pcb* pcb);

// Event callbacks
void async_tcp_recv(struct tcp_pcb* pcb, tcp_recv_fn callback);
void async_tcp_sent(struct tcp_pcb* pcb, tcp_sent_fn callback);
void async_tcp_poll(struct tcp_pcb* pcb, tcp_poll_fn callback, u8_t interval);
void async_tcp_err(struct tcp_pcb* pcb, tcp_err_fn callback);

// DNS callback
err_t async_dns_gethostbyname(const char* hostname, ip_addr_t* addr, dns_found_callback found, void* callback_arg);

#ifdef __cplusplus
}
#endif

#endif // 0
