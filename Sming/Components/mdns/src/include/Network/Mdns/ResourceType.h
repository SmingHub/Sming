#pragma once

#include <WString.h>

/**
 * @brief MDNS resource type identifiers
 * 
 * (name, value, description)
 */
#define MDNS_RESOURCE_TYPE_MAP(XX)                                                                                     \
	XX(A, 0x0001, "32-bit IPv4 address")                                                                               \
	XX(PTR, 0x000C, "Pointer to a canonical name")                                                                     \
	XX(HINFO, 0x000D, "Host Information")                                                                              \
	XX(TXT, 0x0010, "Arbitrary human-readable text")                                                                   \
	XX(AAAA, 0x001C, "128-bit IPv6 address")                                                                           \
	XX(SRV, 0x0021, "Server selection")

namespace mDNS
{
enum class ResourceType : uint16_t {
#define XX(name, value, desc) name = value,
	MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
};

} // namespace mDNS

String toString(mDNS::ResourceType type);
