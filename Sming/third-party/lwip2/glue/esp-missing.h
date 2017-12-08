
#ifndef ESP_MISSING_H
#define ESP_MISSING_H

#include <stdarg.h>
#include <stdint.h>

// these declarations are missing from sdk and used by lwip1.4 from sdk2.0.0

uint32_t r_rand (void);

void* pvPortZalloc (size_t, const char*, unsigned line);
void* pvPortMalloc (size_t xWantedSize, const char* file, unsigned line) __attribute__((malloc, alloc_size(1)));
void vPortFree (void *ptr, const char* file, unsigned line);

struct netif* eagle_lwip_getif (int netif_index);

void ets_intr_lock (void);
void ets_intr_unlock (void);

int ets_vprintf (int (*print_function)(int), const char * format, va_list arg) __attribute__ ((format (printf, 2, 0)));
int ets_sprintf (char *str, const char *format, ...)  __attribute__ ((format (printf, 2, 3)));
int ets_putc(int);

void ets_bzero (void*, size_t);
int ets_memcmp (const void*, const void*, size_t n);
void *ets_memset (void *s, int c, size_t n);
void *ets_memcpy (void *dest, const void *src, size_t n);

//typedef void ETSTimerFunc(void *timer_arg);
//void ets_timer_disarm (ETSTimer *a);
//void ets_timer_arm_new (ETSTimer *a, int b, int c, int isMstimer);
//void ets_timer_setfn (ETSTimer *t, ETSTimerFunc *fn, void *parg);

struct ip_addr;
void wifi_softap_set_station_info (uint8_t* mac, struct ip_addr*);

#define os_intr_lock	ets_intr_lock
#define os_intr_unlock	ets_intr_unlock
#define os_bzero	ets_bzero
#define os_memcmp	ets_memcmp
#define os_memset	ets_memset
#define os_memcpy	ets_memcpy

#endif
