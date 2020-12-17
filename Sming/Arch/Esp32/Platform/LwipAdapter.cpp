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

#if 0

#include "LwipAdapter.h"
#include <Network/TcpConnection.h>
#include "esp_task_wdt.h"
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/priv/tcpip_priv.h"
}

// TODO: Tune these....
#define CONFIG_ASYNC_TCP_RUNNING_CORE 1
#define CONFIG_ASYNC_TCP_USE_WDT 1

typedef enum {
	LWIP_TCP_SENT,
	LWIP_TCP_RECV,
	LWIP_TCP_FIN,
	LWIP_TCP_ERROR,
	LWIP_TCP_POLL,
	LWIP_TCP_CLEAR,
	LWIP_TCP_ACCEPT,
	LWIP_TCP_CONNECTED,
	LWIP_TCP_DNS
} lwip_event_t;

typedef struct {
	lwip_event_t event;
	void* arg;
	union {
		struct {
			tcp_pcb* pcb;
			int8_t err;
		} connected;
		struct {
			int8_t err;
		} error;
		struct {
			tcp_pcb* pcb;
			uint16_t len;
		} sent;
		struct {
			tcp_pcb* pcb;
			pbuf* pb;
			int8_t err;
		} recv;
		struct {
			tcp_pcb* pcb;
			int8_t err;
		} fin;
		struct {
			tcp_pcb* pcb;
		} poll;
		struct {
			tcp_pcb* pcb;
			int8_t err;
		} accept;
		struct {
			const char* name;
			ip_addr_t addr;
		} dns;
	};
} lwip_event_packet_t;

struct DnsLookup {
	TcpConnection* con;
	int port;
};

typedef struct {
	TcpConnection* connection;
	uint8_t closed; // 0 - open, 1 - closed
	struct {
		tcp_connected_fn callback;
	} connect;
	struct {
		tcp_accept_fn callback;
	} accept;
	struct {
		tcp_recv_fn callback;
	} receive;
	struct {
		tcp_sent_fn callback;
	} sent;
	struct {
		tcp_poll_fn callback;
	} poll;
	struct {
		tcp_err_fn callback;
	} error;
	struct {
		dns_found_callback callback;
		DnsLookup* lookup;
	} dns;
	// TODO: ...
} tcp_api_arg_t;

typedef struct {
	struct tcpip_api_call_data call;
	tcp_pcb* pcb;
	int8_t err;
	union {
		struct {
			const char* data;
			size_t size;
			uint8_t apiflags;
		} write;
		size_t received;
		struct {
			const ip_addr_t* addr;
			uint16_t port;
			tcp_connected_fn cb;
		} connect;
		struct {
			const ip_addr_t* addr;
			uint16_t port;
		} bind;
		uint8_t backlog;
	};
} tcp_api_call_t;

// Event Logic
static xQueueHandle _async_queue;
static TaskHandle_t _async_service_task_handle = NULL;

// TODO: remove the slots..
//SemaphoreHandle_t _slots_lock;
//const int _number_of_closed_slots = CONFIG_LWIP_MAX_ACTIVE_TCP;
//static uint32_t _closed_slots[_number_of_closed_slots];
//static uint32_t _closed_index = []() {
//	_slots_lock = xSemaphoreCreateBinary();
//	xSemaphoreGive(_slots_lock);
//	for(int i = 0; i < _number_of_closed_slots; ++i) {
//		_closed_slots[i] = 1;
//	}
//	return 1;
//}();

static inline bool _init_async_event_queue()
{
	if(!_async_queue) {
		_async_queue = xQueueCreate(32, sizeof(lwip_event_packet_t*));
		if(!_async_queue) {
			return false;
		}
	}
	return true;
}

static inline bool _send_async_event(lwip_event_packet_t** e)
{
	return _async_queue && xQueueSend(_async_queue, e, portMAX_DELAY) == pdPASS;
}

static inline bool _prepend_async_event(lwip_event_packet_t** e)
{
	return _async_queue && xQueueSendToFront(_async_queue, e, portMAX_DELAY) == pdPASS;
}

static inline bool _get_async_event(lwip_event_packet_t** e)
{
	return _async_queue && xQueueReceive(_async_queue, e, portMAX_DELAY) == pdPASS;
}

static bool _remove_events_with_arg(void* arg)
{
	lwip_event_packet_t* first_packet = NULL;
	lwip_event_packet_t* packet = NULL;

	if(!_async_queue) {
		return false;
	}
	//figure out which is the first packet so we can keep the order
	while(!first_packet) {
		if(xQueueReceive(_async_queue, &first_packet, 0) != pdPASS) {
			return false;
		}
		//discard packet if matching
		if((int)first_packet->arg == (int)arg) {
			free(first_packet);
			first_packet = NULL;
			//return first packet to the back of the queue
		} else if(xQueueSend(_async_queue, &first_packet, portMAX_DELAY) != pdPASS) {
			return false;
		}
	}

	while(xQueuePeek(_async_queue, &packet, 0) == pdPASS && packet != first_packet) {
		if(xQueueReceive(_async_queue, &packet, 0) != pdPASS) {
			return false;
		}
		if((int)packet->arg == (int)arg) {
			free(packet);
			packet = NULL;
		} else if(xQueueSend(_async_queue, &packet, portMAX_DELAY) != pdPASS) {
			return false;
		}
	}
	return true;
}

static void _handle_async_event(lwip_event_packet_t* e)
{
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(e->arg);

	if(e->arg == NULL) {
		// do nothing when arg is NULL
		debug_d("event (%d) arg == NULL(org: 0x%08x): 0x%08x, Pcb State: %d", e->event, e->arg, e->recv.pcb, e->recv.pcb->state);
		if(e->event == LWIP_TCP_POLL) {
			debug_d("Close connection. Pcb: 0x%08x", e->poll.pcb);
			async_tcp_close(e->poll.pcb); // close the connection
		}

	} else if(e->event == LWIP_TCP_CLEAR) {
		_remove_events_with_arg(e->arg);
	} else if(e->event == LWIP_TCP_RECV) {
		debug_d("-R: Pcb: 0x%08x, Arg: 0x%08x, Err: %d", e->recv.pcb, e->arg, e->recv.err);
		if(arg->receive.callback) {
			arg->receive.callback(arg->connection, e->recv.pcb, e->recv.pb, e->recv.err);
		}
	} else if(e->event == LWIP_TCP_FIN) {
		debug_d("-F: Pcb: 0x%08x, Arg: 0x%08x", e->fin.pcb, e->arg);
		debug_d("Close connection. Pcb: 0x%08x", e->fin.pcb);
		async_tcp_close(e->fin.pcb); // close the connection
	} else if(e->event == LWIP_TCP_SENT) {
		debug_d("-S: Pcb: 0x%08x, Arg: 0x%08x, Callback: 0x%08x", e->sent.pcb, e->arg, arg->sent.callback);
		if(arg->sent.callback) {
			arg->sent.callback(arg->connection, e->sent.pcb, e->sent.len);
		}
	} else if(e->event == LWIP_TCP_POLL) {
		debug_d("-P: Pcb: 0x%08x, Arg: 0x%08x, Callback: 0x%08x", e->poll.pcb, e->arg, arg->poll.callback);
		if(arg->poll.callback) {
			arg->poll.callback(arg->connection, e->poll.pcb);
		}
	} else if(e->event == LWIP_TCP_ERROR) {
		debug_d("-E: Arg: 0x%08x, Error: %d", e->arg, e->error.err);
		if(arg->error.callback) {
			arg->error.callback(arg->connection, e->error.err);
		}
	} else if(e->event == LWIP_TCP_CONNECTED) {
		debug_d("-C: Pcb: 0x%08x, Arg: 0x%08x, Callback: 0x%08x, Err: %d", e->connected.pcb, e->arg, arg->connect.callback, e->connected.err);
		if(arg->connect.callback) {
			arg->connect.callback(arg->connection, e->connected.pcb, e->connected.err);
		}
	} else if(e->event == LWIP_TCP_ACCEPT) {
		debug_d("-A: Pcb: 0x%08x, Arg: 0x%08x, Callback: 0x%08x", e->accept.pcb, e->arg, arg->accept.callback);
		if(arg->accept.callback) {
			arg->accept.callback(arg->connection, e->accept.pcb, e->accept.err);
		}
	} else if(e->event == LWIP_TCP_DNS) {
		debug_d("D: 0x%08x %s = %s", e->arg, e->dns.name, ipaddr_ntoa(&e->dns.addr));
		if(arg->dns.callback) {
			arg->dns.callback(e->dns.name, &e->dns.addr, arg->dns.lookup);
		}
		delete arg;
	}
	free((void*)(e));
}

static void _async_service_task(void* pvParameters)
{
	lwip_event_packet_t* packet = NULL;
	for(;;) {
		if(_get_async_event(&packet)) {
#if CONFIG_ASYNC_TCP_USE_WDT
			if(esp_task_wdt_add(NULL) != ESP_OK) {
				debug_e("Failed to add async task to WDT");
			}
#endif
			_handle_async_event(packet);
#if CONFIG_ASYNC_TCP_USE_WDT
			if(esp_task_wdt_delete(NULL) != ESP_OK) {
				debug_e("Failed to remove loop task from WDT");
			}
#endif
		}
	}
	vTaskDelete(NULL);
	_async_service_task_handle = NULL;
}
/*
static void _stop_async_task(){
    if(_async_service_task_handle){
        vTaskDelete(_async_service_task_handle);
        _async_service_task_handle = NULL;
    }
}
*/
static bool _start_async_task()
{
	if(!_init_async_event_queue()) {
		return false;
	}
	if(!_async_service_task_handle) {
		xTaskCreatePinnedToCore(_async_service_task, "async_tcp", 8192 * 2, NULL, 3, &_async_service_task_handle,
								CONFIG_ASYNC_TCP_RUNNING_CORE);
		if(!_async_service_task_handle) {
			return false;
		}
		debug_d("Starting Async Event Queue.");
	}
	return true;
}

// Low-Level LwIP Functions
static int8_t _tcp_clear_events(void* arg)
{
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	e->event = LWIP_TCP_CLEAR;
	e->arg = arg;
	if(!_prepend_async_event(&e)) {
		free((void*)(e));
	}
	return ERR_OK;
}

static int8_t _tcp_connected(void* arg, tcp_pcb* pcb, int8_t err)
{
	debug_d("+C: 0x%08x", pcb);
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));

	e->event = LWIP_TCP_CONNECTED;
	e->arg = arg;
	e->connected.pcb = pcb;
	e->connected.err = err;
	if(!_prepend_async_event(&e)) {
		free((void*)(e));
	}
	return ERR_OK;
}

static int8_t _tcp_poll(void* arg, struct tcp_pcb* pcb)
{
	debug_d("+P: 0x%08x", pcb);
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	e->event = LWIP_TCP_POLL;
	e->arg = arg;
	e->poll.pcb = pcb;
	if(!_send_async_event(&e)) {
		free((void*)(e));
	}
	return ERR_OK;
}

static int8_t _tcp_recv(void* arg, struct tcp_pcb* pcb, struct pbuf* pb, int8_t err)
{
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	e->arg = arg;
	if(pb) {
		debug_d("+R: 0x%08x", pcb);
		e->event = LWIP_TCP_RECV;
		e->recv.pcb = pcb;
		e->recv.pb = pb;
		e->recv.err = err;
	} else {
		debug_d("+F: 0x%08x", pcb);
		e->event = LWIP_TCP_FIN;
		e->fin.pcb = pcb;
		e->fin.err = err;
		//        //close the PCB in LwIP thread
		//        // TODO:
		////        TcpConnection::_s_lwip_fin(e->arg, e->fin.pcb, e->fin.err);
	}
	if(!_send_async_event(&e)) {
		free((void*)(e));
	}
	return ERR_OK;
}

static int8_t _tcp_sent(void* arg, struct tcp_pcb* pcb, uint16_t len)
{
	debug_d("+S: 0x%08x", pcb);
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	e->event = LWIP_TCP_SENT;
	e->arg = arg;
	e->sent.pcb = pcb;
	e->sent.len = len;
	if(!_send_async_event(&e)) {
		free((void*)(e));
	}
	return ERR_OK;
}

static void _tcp_error(void* arg, int8_t err)
{
	debug_d("+E: 0x%08x", arg);
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	e->event = LWIP_TCP_ERROR;
	e->arg = arg;
	e->error.err = err;
	if(!_send_async_event(&e)) {
		free((void*)(e));
	}
}

static void _tcp_dns_found(const char* name, const ip_addr* ipaddr, void* arg)
{
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	debug_d("+DNS: name=%s ipaddr=0x%08x arg=%x", name, ipaddr, arg);
	e->event = LWIP_TCP_DNS;
	e->arg = arg;
	e->dns.name = name;
	if(ipaddr) {
		memcpy(&e->dns.addr, ipaddr, sizeof(struct ip_addr));
	} else {
		memset(&e->dns.addr, 0, sizeof(e->dns.addr));
	}
	if(!_send_async_event(&e)) {
		free((void*)(e));
	}
}

//Used to switch out from LwIP thread
static int8_t _tcp_accept(void* arg, struct tcp_pcb* newpcb, err_t err)
{
	debug_d("+A: arg: 0x%08x, new pcb: 0x%08x, error: %d", arg, newpcb, err);
	lwip_event_packet_t* e = (lwip_event_packet_t*)malloc(sizeof(lwip_event_packet_t));
	e->event = LWIP_TCP_ACCEPT;
	e->arg = arg;
	e->accept.pcb = newpcb;
	e->accept.err = err;
	if(!_prepend_async_event(&e)) {
		free((void*)(e));
	}
	return ERR_OK;
}

/*
 * TCP/IP API Calls
 * */

static err_t _tcp_output_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = ERR_CONN;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg->pcb->callback_arg);
	if(arg != nullptr && arg->closed) {
		return msg->err;
	}
	msg->err = tcp_output(msg->pcb);
	return msg->err;
}

static esp_err_t _tcp_output(tcp_pcb* pcb)
{
	if(!pcb) {
		return ERR_CONN;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;
	tcpip_api_call(_tcp_output_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_write_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = ERR_CONN;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg->pcb->callback_arg);
	if(arg != nullptr && arg->closed) {
		return msg->err;
	}
	msg->err = tcp_write(msg->pcb, msg->write.data, msg->write.size, msg->write.apiflags);
	return msg->err;
}

static esp_err_t _tcp_write(tcp_pcb* pcb, const char* data, size_t size, uint8_t apiflags)
{
	if(!pcb) {
		return ERR_CONN;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;

	msg.write.data = data;
	msg.write.size = size;
	msg.write.apiflags = apiflags;
	tcpip_api_call(_tcp_write_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_recved_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = ERR_CONN;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg->pcb->callback_arg);
	if(arg != nullptr && arg->closed) {
		return msg->err;
	}

	tcp_recved(msg->pcb, msg->received);
	return msg->err;
}

static esp_err_t _tcp_recved(tcp_pcb* pcb, size_t len)
{
	if(!pcb) {
		return ERR_CONN;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;

	msg.received = len;
	tcpip_api_call(_tcp_recved_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_close_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = ERR_CONN;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg->pcb->callback_arg);
	if(arg != nullptr) {
		if(arg->closed) {
			return msg->err;
		}
		arg->closed = 1;
	}
	msg->err = tcp_close(msg->pcb);
	_tcp_clear_events(msg->pcb);
	return msg->err;
}

static esp_err_t _tcp_close(tcp_pcb* pcb)
{
	if(!pcb) {
		return ERR_CONN;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;

	tcpip_api_call(_tcp_close_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_abort_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = ERR_CONN;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg->pcb->callback_arg);
	if(arg != nullptr) {
		if(arg->closed) {
			return msg->err;
		}
		arg->closed = 1;
	}

	tcp_abort(msg->pcb);
	return msg->err;
}

static esp_err_t _tcp_abort(tcp_pcb* pcb)
{
	if(!pcb) {
		return ERR_CONN;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;

	tcpip_api_call(_tcp_abort_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_connect_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = tcp_connect(msg->pcb, msg->connect.addr, msg->connect.port, _tcp_connected);
	return msg->err;
}

static esp_err_t _tcp_connect(tcp_pcb* pcb, const ip_addr_t* addr, uint16_t port, tcp_connected_fn cb)
{
	if(!pcb) {
		return ESP_FAIL;
	}

	tcp_api_call_t msg;

	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	if(arg != nullptr) {
		debug_d("Connecting ...");
		arg->closed = 0;
		arg->connect.callback = cb;
	}

	msg.pcb = pcb;

	msg.connect.addr = addr;
	msg.connect.port = port;
	msg.connect.cb = _tcp_connected;
	tcpip_api_call(_tcp_connect_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_bind_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = tcp_bind(msg->pcb, msg->bind.addr, msg->bind.port);
	return msg->err;
}

static esp_err_t _tcp_bind(tcp_pcb* pcb, const ip_addr_t* addr, uint16_t port)
{
	if(!pcb) {
		return ESP_FAIL;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg.pcb->callback_arg);
	if(arg != nullptr) {
		arg->closed = 0;
	}
	msg.bind.addr = addr;
	msg.bind.port = port;
	tcpip_api_call(_tcp_bind_api, (struct tcpip_api_call_data*)&msg);
	return msg.err;
}

static err_t _tcp_listen_api(struct tcpip_api_call_data* api_call_msg)
{
	tcp_api_call_t* msg = (tcp_api_call_t*)api_call_msg;
	msg->err = 0;
	msg->pcb = tcp_listen_with_backlog(msg->pcb, msg->backlog);
	return msg->err;
}

static tcp_pcb* _tcp_listen_with_backlog(tcp_pcb* pcb, uint8_t backlog)
{
	if(!pcb) {
		return NULL;
	}
	tcp_api_call_t msg;
	msg.pcb = pcb;
	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(msg.pcb->callback_arg);
	if(arg != nullptr) {
		arg->closed = 0;
	}
	msg.backlog = backlog ? backlog : TCP_DEFAULT_LISTEN_BACKLOG;
	tcpip_api_call(_tcp_listen_api, (struct tcpip_api_call_data*)&msg);
	return msg.pcb;
}

// High Level Functions

tcp_pcb* async_tcp_new()
{
	return tcp_new_ip_type(IPADDR_TYPE_V4);
}

void async_tcp_arg(struct tcp_pcb* pcb, void* arg)
{
	if(pcb == nullptr) {
		return;
	}

	tcp_api_arg_t* apiArgument = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	// delete the current argument if this is an api argument
	if(apiArgument != nullptr) {
		delete apiArgument;
		apiArgument = nullptr;
	}

	TcpConnection* connectionArg = reinterpret_cast<TcpConnection*>(arg);
	if(connectionArg != nullptr) {
		// we have the right argument, lets wrap it
		if(apiArgument == nullptr) {
			apiArgument = new tcp_api_arg_t();
			memset(apiArgument, 0, sizeof(tcp_api_arg_t));
		}
		apiArgument->connection = connectionArg;
		debug_d("Arg: Set wrapped arg. Pcb: 0x%08x, arg: 0x%08x, connection: 0x%08x", pcb, apiArgument, arg);
		tcp_arg(pcb, apiArgument);
		return;
	}

	// all other arguments are set as usual
	debug_d("Arg: Set unknown arg. Pcb: 0x%08x, arg: 0x%08x", pcb, arg);
	tcp_arg(pcb, arg);
}

err_t async_tcp_connect(tcp_pcb* pcb, const ip_addr_t* ipaddr, u16_t port, tcp_connected_fn connected)
{
	if(!_start_async_task()) {
		return ERR_ABRT;
	}

	debug_d("Connecting: Pcb: 0x%08x", pcb);

	return _tcp_connect(pcb, ipaddr, port, connected);
}

tcp_pcb* async_tcp_listen_with_backlog(struct tcp_pcb* pcb, u8_t backlog)
{
	if(!_start_async_task()) {
		return nullptr;
	}

	return _tcp_listen_with_backlog(pcb, backlog);
}

err_t async_tcp_bind(struct tcp_pcb* pcb, const ip_addr_t* ipaddr, u16_t port)
{
	if(!_start_async_task()) {
		return ERR_ABRT;
	}

	return _tcp_bind(pcb, ipaddr, port);
}

void async_tcp_accept(struct tcp_pcb* pcb, tcp_accept_fn callback)
{
	if(pcb == nullptr) {
		return;
	}

	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	if(arg == nullptr) {
		debug_d("Set to direct accept callback: Pcb: 0x%08x, Arg: 0x%08x, State: %d.", pcb, pcb->callback_arg, pcb->state);
		tcp_accept(pcb, callback);
		return;
	}

	debug_d("Accept: Pcb: 0x%08x, Callback: 0x%08x", pcb, callback);

	arg->accept.callback = callback;
	tcp_accept(pcb, _tcp_accept);
}

err_t async_tcp_write(struct tcp_pcb* pcb, const void* dataptr, u16_t len, u8_t apiflags)
{
	return _tcp_write(pcb, (const char*)dataptr, len, apiflags);
}

void async_tcp_recved(struct tcp_pcb* pcb, u16_t len)
{
	_tcp_recved(pcb, len);
}

err_t async_tcp_output(struct tcp_pcb* pcb)
{
	return _tcp_output(pcb);
}

void async_tcp_abort(struct tcp_pcb* pcb)
{
	_tcp_abort(pcb);
}

err_t async_tcp_close(struct tcp_pcb* pcb)
{
	return _tcp_close(pcb);
}

void async_tcp_recv(struct tcp_pcb* pcb, tcp_recv_fn callback)
{
	if(pcb == nullptr) {
		return;
	}

	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	if(arg == nullptr) {
		debug_d("Set to direct recv callback: Pcb: 0x%08x, Arg: 0x%08x, State: %d.", pcb, pcb->callback_arg, pcb->state);
		tcp_recv(pcb, callback);
		return;
	}

	debug_d("Receive: Pcb: 0x%08x, Callback: 0x%08x", pcb, callback);

	arg->receive.callback = callback;
	tcp_recv(pcb, _tcp_recv);
}

void async_tcp_sent(struct tcp_pcb* pcb, tcp_sent_fn callback)
{
	if(pcb == nullptr) {
		return;
	}

	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	if(arg == nullptr) {
		debug_d("Set to direct sent callback: Pcb: 0x%08x, Arg: 0x%08x, State: %d.", pcb, pcb->callback_arg, pcb->state);
		tcp_sent(pcb, callback);
		return;
	}

	debug_d("Sent: Pcb: 0x%08x, Callback: 0x%08x", pcb, callback);

	arg->sent.callback = callback;
	tcp_sent(pcb, _tcp_sent);
}

void async_tcp_poll(struct tcp_pcb* pcb, tcp_poll_fn callback, u8_t interval)
{
	if(pcb == nullptr) {
		return;
	}

	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	if(arg == nullptr) {
		debug_d("Set to direct poll callback: Pcb: 0x%08x, Arg: 0x%08x, State: %d.", pcb, pcb->callback_arg, pcb->state);
		tcp_poll(pcb, callback, interval);
		return;
	}

	debug_d("Poll: Pcb: 0x%08x, Callback: 0x%08x, Interval: %d", pcb, callback, interval);

	arg->poll.callback = callback;
	tcp_poll(pcb, _tcp_poll, interval);
}

void async_tcp_err(struct tcp_pcb* pcb, tcp_err_fn callback)
{
	if(pcb == nullptr) {
		return;
	}

	tcp_api_arg_t* arg = reinterpret_cast<tcp_api_arg_t*>(pcb->callback_arg);
	if(arg == nullptr) {
		debug_d("Set to direct error callback: Pcb: 0x%08x, Arg: 0x%08x, State: %d.", pcb, pcb->callback_arg, pcb->state);
		tcp_err(pcb, callback);
		return;
	}

	debug_d("Error: Pcb: 0x%08x, Callback: 0x%08x", pcb, callback);

	arg->error.callback = callback;
	tcp_err(pcb, _tcp_error);
}

err_t async_dns_gethostbyname(const char* hostname, ip_addr_t* addr, dns_found_callback found, void* callback_arg)
{
	DnsLookup* dnsLookup = reinterpret_cast<DnsLookup*>(callback_arg);
	if(dnsLookup == nullptr) {
		// This should not happen
		return ERR_ABRT;
	}

	tcp_api_arg_t* arg = new tcp_api_arg_t();
	if(arg == nullptr) {
		// This should not happen
		return ERR_ABRT;
	}

	if(!_start_async_task()) {
		delete arg;
		return ERR_ABRT;
	}

	arg->dns.callback = found;
	arg->dns.lookup = dnsLookup;
	return dns_gethostbyname(hostname, addr, _tcp_dns_found, arg);
}

#endif // 0
