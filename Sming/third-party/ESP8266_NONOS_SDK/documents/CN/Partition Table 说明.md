<a name="CN-Doc"></a>
## 1. Partition Table 说明
从 ESP8266\_NonOS\_SDK_V3.0 起，增加了 partition table（参见 `include/user_interface.h` 中 的 `partition_item_t` 结构体），用于管理各个 flash 分区，用户可以自行调整部分 partition 地址及大小。  
使用 partition table 的示例可参考 `ESP8266_NONOS_SDK/examples/IoT_Demo/user/user_main.c`，注意点如下：

- 用户必须在 `user_pre_init` 中调用 `system_partition_table_regist` 函数注册 partition table：
    - `system_partition_table_regist` 注册时，会检查各个 partition 区域是否有覆盖，如果有覆盖，则返回 false，用户应重新调整 partition。  
    - `system_partition_table_regist` 第三个参数传递的 map 方式必须和编译烧录时选择的 map 一致，否则将会启动异常，建议可以直接传入宏 `SPI_FLASH_SIZE_MAP`。
- boot.bin 必须烧录到 0x0，不允许改动。
- user1.bin 必须烧录到 0x1000，不允许改动。
- user2.bin：
    - 对于 512+512 map，必须烧录到 0x81000；
    - 对于 1024+1024 map，必须烧录到 0x101000；
    - 不允许改动。  

ESP8266\_NonOS\_SDK 定义的 partition 类型如下，位于 `include/user_interface.h` 中：

```  
typedef enum {
    SYSTEM_PARTITION_INVALID = 0,
    SYSTEM_PARTITION_BOOTLOADER,            /* user can't modify this partition address */
    SYSTEM_PARTITION_OTA_1,                 /* user can't modify this partition address, but can modify its size */
    SYSTEM_PARTITION_OTA_2,                 /* user can't modify this partition address, but can modify its size */
    SYSTEM_PARTITION_RF_CAL,                /* user must define this partition */
    SYSTEM_PARTITION_PHY_DATA,              /* user must define this partition */
    SYSTEM_PARTITION_SYSTEM_PARAMETER,      /* user must define this partition */
    SYSTEM_PARTITION_AT_PARAMETER,
    SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY,
    SYSTEM_PARTITION_SSL_CLIENT_CA,
    SYSTEM_PARTITION_SSL_SERVER_CERT_PRIVKEY,
    SYSTEM_PARTITION_SSL_SERVER_CA,
    SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY,
    SYSTEM_PARTITION_WPA2_ENTERPRISE_CA,
    
    SYSTEM_PARTITION_CUSTOMER_BEGIN = 100,  /* user can define partition after here */
    SYSTEM_PARTITION_MAX
} partition_type_t;
```
* `SYSTEM_PARTITION_RF_CAL`、`SYSTEM_PARTITION_PHY_DATA` 和 `SYSTEM_PARTITION_SYSTEM_PARAMETER` 必须在 partition table 中定义，位置可以自己指定。
* 用户如需增加自定义 partition，从 `SYSTEM_PARTITION_CUSTOMER_BEGIN` 之后增加即可。并且，在应用程序的 partition table 中定义，这样 SDK 检查时，将一并检查自定义的 partition。


## 2. Non-FOTA 应用
用户如需使用 Non-FOTA 的方式，可以从 `SYSTEM_PARTITION_CUSTOMER_BEGIN` 后面定义 partition，如：

```
#define EAGLE_FLASH_BIN_ADDR				（SYSTEM_PARTITION_CUSTOMER_BEGIN + 1）
#define EAGLE_IROM0TEXT_BIN_ADDR			（SYSTEM_PARTITION_CUSTOMER_BEGIN + 2）

static const partition_item_t partition_table[] = {
    { EAGLE_FLASH_BIN_ADDR, 	0x00000, 0x10000},
    { EAGLE_IROM0TEXT_BIN_ADDR, 0x10000, 0x60000},
    { SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000},
    { SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER,SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(partition_table, sizeof(partition_table)/sizeof(partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

```

## 3. AT: 512+512 map
受限于 Flash 和 bin 文件的大小，从 ESP8266\_NonOS\_SDK_V3.0 起，AT 默认不再支持 512+512 的 map 方式。  
如果用户需要支持 512+512 map，可以自行修改 AT 应用，调整 partition 分区，方法如下：  

* 删减功能，确保 AT 编译生成的 user.bin 文件大小不能超过 AT 应用中自定义的 `SYSTEM_PARTITION_OTA_SIZE`。例如，由使用 libmbedtls 改为使用 libssl，或者移除 SmartConfig 功能等。
* 自定义 partition table，例如:  

```
#define SYSTEM_PARTITION_OTA_SIZE			                        0x6A000  // 请确保编译生成的 bin 文件不超过此值
#define SYSTEM_PARTITION_OTA_2_ADDR		                        0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR		                        0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR	                        0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR                  0xfd000
#define SYSTEM_PARTITION_AT_PARAMETER_ADDR.                     0x7d000
#define SYSTEM_PARTITION_SSL_CLIENT_CERT_PRIVKEY_ADDR           0x7c000
#define SYSTEM_PARTITION_SSL_CLIENT_CA_ADDR.                    0x7b000
#define SYSTEM_PARTITION_WPA2_ENTERPRISE_CERT_PRIVKEY_ADDR      0x7a000
#define SYSTEM_PARTITION_WPA2_ENTERPRISE_CA_ADDR                0x79000
```
