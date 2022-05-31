/**
 * Additional settings for the win32 port.
 */

#define PCAPIF_LIB_QUIET

#define PCAPIF_HANDLE_LINKSTATE 0
#define PCAPIF_FILTER_GROUP_ADDRESSES 0

/* remember to change this MAC address to suit your needs!
   the last octet will be increased by netif->num for each netif */
#define LWIP_MAC_ADDR_BASE                                                                                             \
	{                                                                                                                  \
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05                                                                             \
	}
