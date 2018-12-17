# ESP8266_NONOS_SDK

All documentations @ http://espressif.com/en/support/download/documents?keys=&field_type_tid%5B%5D=14


## Notes ##
Please add `user_pre_init()` in your project, which will be called before `user_init()`. And you MUST call `system_partition_table_regist()` in `user_pre_init` to register your project partition table.  

The following partition address CAN NOT be modified, and you MUST give the correct address. They are retated to the flash map, please refer to [ESP8266 SDK Getting Started Guide](https://www.espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_en.pdf) or [ESP8266 SDK 入门指南](https://www.espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_cn.pdf).  
  
- SYSTEM\_PARTITION\_BOOTLOADER  
- SYSTEM\_PARTITION\_OTA_1  
- SYSTEM\_PARTITION\_OTA_2  
- SYSTEM\_PARTITION\_SYSTEM_PARAMETER  

If you donot use Non-FOTA bin, eagle.irom0.text.bin and irom0.text MUST be downloaded the fixed address, which also can be found in [ESP8266 SDK Getting Started Guide](https://www.espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_en.pdf) or [ESP8266 SDK 入门指南](https://www.espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_cn.pdf), and you can define their partition type after `SYSTEM_PARTITION_CUSTOMER_BEGIN`.