
#include "include/heap.h"
#include "esp_system.h"

uint32_t system_get_free_heap_size(void)
{
	return esp_get_free_heap_size();
}
