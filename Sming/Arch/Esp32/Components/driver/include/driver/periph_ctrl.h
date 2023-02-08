#pragma once

#include <esp_idf_version.h>

#if ESP_IDF_VERSION_MAJOR < 5
#include_next <driver/periph_ctrl.h>
#else
#include <esp_private/periph_ctrl.h>
#endif
