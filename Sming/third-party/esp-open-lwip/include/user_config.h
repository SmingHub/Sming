#ifndef _USER_CONFIG_LWIP_
#define _USER_CONFIG_LWIP_

#ifdef __cplusplus
extern "C" {
#endif

typedef signed short        sint16_t;

void *ets_bzero(void *block, size_t size);
bool ets_post(uint32_t prio, ETSSignal sig, ETSParam par);
void ets_task(ETSTask task, uint32_t prio, ETSEvent * queue, uint8 qlen);

void system_pp_recycle_rx_pkt(void *eb);

#ifdef __cplusplus
}
#endif

#endif /*_USER_CONFIG_LWIP_*/
