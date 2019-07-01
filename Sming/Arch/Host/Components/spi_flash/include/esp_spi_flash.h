#define SPI_FLASH_SEC_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

#include <c_types.h>

uint32_t spi_flash_get_id(void);

#ifdef __cplusplus
}
#endif

#include_next <esp_spi_flash.h>
