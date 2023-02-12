#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#include_next <driver/spi_master.h>

#include <esp_idf_version.h>
#if ESP_IDF_VERSION_MAJOR >= 5
#include <esp_memory_utils.h>
#endif

#pragma GCC diagnostic pop
