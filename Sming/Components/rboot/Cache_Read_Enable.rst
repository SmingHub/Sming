ESP8266 Cache_Read_Enable
=========================

12th June 2015 Richard

Since I haven’t seen this documented anywhere, here is my attempt to explain the Cache_Read_Enable function.
Valid values and what they do (at a register level) are from decompiling the code.
The outcome of those values is based on my own experimentation so my descriptions and explanations
may be silly but they currently fit the observed results.

void Cache_Read_Enable(uint8 odd_even, uint8 mb_count, unt8 no_idea);

Valid values for odd_even
   0 – clears bits 24 & 25 of control register 0x3FF0000C
   1 – clears bit 24, sets bit 25
   other – clears bit 25, sets bit 24

Function of odd_even
   0 – allows access to even numbered mb
   1 – allow access to odd numbered mb
   other – appears to do the same as 1, there must be a difference but I haven’t worked out what it is

Valid values for mb_count
   0-7 – set bits 16, 17 & 18 of control register 0x3FF0000C

Function of mb_count
   Which odd or even bank to map (according to odd_even option)
      e.g. mb_count = 0, odd_even = 0 -> map first 8Mbit of flash
      e.g. mb_count = 0, odd_even = 1 -> map second 8Mbit of flash
      e.g. mb_count = 1, odd_even = 0 -> map third 8Mbit of flash
      e.g. mb_count = 1, odd_even = 1 -> map fourth 8Mbit of flash

Valid values for no_idea
   0 – sets bit 3 of 0x3FF00024
   1 – sets bit 26 of 0x3FF0000C and sets bits 3 & 4 of 0x3FF00024

Function of no_idea
   The clue is in the name, I can’t work out what this does from my experiments, but the SDK always sets this to 1.


**Source: https://richard.burtons.org/2015/06/12/esp8266-cache_read_enable/**


13/11/2019 @author mikee47 UPDATE
---------------------------------

.. highlight:: c++

Ref. RTOS SDK source ``bootloader_support/bootloader_utility.c``,
function ``bootloader_utility_load_image()``::

   extern void Cache_Read_Enable(uint8_t map, uint8_t p, uint8_t v);
   ...
   Cache_Read_Enable(map, 0, SOC_CACHE_SIZE);
   
Where SOC_CACHE_SIZE is defined as::

   #ifdef CONFIG_SOC_FULL_ICACHE
   #define SOC_CACHE_SIZE 1 // 32KB
   #else
   #define SOC_CACHE_SIZE 0 // 16KB
   #endif


06/04/2021 @author mikee47 UPDATE
---------------------------------

RTOS SDK code has changed, now see usage in ``esp_fast_boot.c``.
Call looks like this::

   Cache_Read_Enable(sub_region, region, SOC_CACHE_SIZE);

See ``esp_fast_boot_restart()``. Code (rearranged) looks like this::

   extern void pm_goto_rf_on(void);
   extern void clockgate_watchdog(int on);

   int esp_fast_boot_restart(void)
   {
      const esp_partition_t* to_boot = esp_ota_get_boot_partition();
      if (!to_boot) {
         ESP_LOGI(TAG, "no OTA boot partition");
         to_boot = esp_ota_get_running_partition();
         if (!to_boot) {
               ESP_LOGE(TAG, "ERROR: Fail to get running partition");
               return -EINVAL;
         }
      }

      uint32_t image_start = to_boot->address;
      uint32_t image_size = to_boot->size - 4;

      esp_image_header_t image;
      int ret = spi_flash_read(image_start, &image, sizeof(esp_image_header_t));
      if (ret != ESP_OK) {
         ESP_LOGE(TAG, "ERROR: Fail to read image head from spi flash error=%d", ret);
         return -EIO;
      }

      uint32_t image_entry = image.entry_addr;
      uint8_t region;
      if (image_start < 0x200000) {
         region = 0;
      } else if (image_start < 0x400000) {
         region = 1;
      } else if (image_start < 0x600000) {
         region = 2;
      } else if (image_start < 0x800000) {
         region = 3;
      } else {
         ESP_LOGE(TAG, "ERROR: App bin error, start_addr 0x%08x image_len %d\n", image_start, image_size);
         return -EINVAL;
      }

      uint8_t sub_region;
      uint32_t image_mask =  image_start & 0x1fffff;
      if (image_mask < 0x100000) {
         sub_region = 0;
      } else {
         sub_region = 1;
      }

      pm_goto_rf_on();
      clockgate_watchdog(0);
      REG_WRITE(0x3ff00018, 0xffff00ff);
      SET_PERI_REG_MASK(0x60000D48, BIT1);
      CLEAR_PERI_REG_MASK(0x60000D48, BIT1);

      REG_WRITE(INT_ENA_WDEV, 0);
      _xt_isr_mask(UINT32_MAX);

      const uint32_t sp = DRAM_BASE + DRAM_SIZE - 16;

      Cache_Read_Disable();
      Cache_Read_Enable(sub_region, region, SOC_CACHE_SIZE);

      __asm__ __volatile__(
         "mov    a1, %0\n"
         : : "a"(sp) : "memory"
      );

      void (*user_start)(size_t start_addr);
      user_start = (void *)entry_addr;
      user_start(image_start);
   }
