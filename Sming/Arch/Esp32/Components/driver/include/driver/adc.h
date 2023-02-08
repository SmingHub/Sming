#pragma once

#include <esp_idf_version.h>

#if ESP_IDF_VERSION_MAJOR < 5
#include_next <driver/adc.h>
#else
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_continuous.h>
#endif
