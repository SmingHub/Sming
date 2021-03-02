#include "include/Network/Mdns/ResourceType.h"

String toString(mDNS::ResourceType type)
{
	switch(type) {
#define XX(name, value, desc)                                                                                          \
	case mDNS::ResourceType::name:                                                                                     \
		return F(#name);
		MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
	default:
		return String(unsigned(type));
	}
}
