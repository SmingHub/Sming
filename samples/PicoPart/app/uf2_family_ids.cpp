#include "uf2_family_ids.h"
#include <pico.h>
#include <boot/picobin.h>
#include <boot/uf2.h>

namespace UF2::Family
{
void parseFlags(CStringArray& ids, uint32_t flags)
{
	if(flags & PICOBIN_PARTITION_FLAGS_ACCEPTS_DEFAULT_FAMILY_ABSOLUTE_BITS) {
		ids.add("absolute");
	}
	if(flags & PICOBIN_PARTITION_FLAGS_ACCEPTS_DEFAULT_FAMILY_RP2040_BITS) {
		ids.add("rp2040");
	}
	if(flags & PICOBIN_PARTITION_FLAGS_ACCEPTS_DEFAULT_FAMILY_RP2350_ARM_S_BITS) {
		ids.add("rp2350-arm-s");
	}
	if(flags & PICOBIN_PARTITION_FLAGS_ACCEPTS_DEFAULT_FAMILY_RP2350_ARM_NS_BITS) {
		ids.add("rp2350-arm-ns");
	}
	if(flags & PICOBIN_PARTITION_FLAGS_ACCEPTS_DEFAULT_FAMILY_RP2350_RISCV_BITS) {
		ids.add("rp2350-riscv");
	}
	if(flags & PICOBIN_PARTITION_FLAGS_ACCEPTS_DEFAULT_FAMILY_DATA_BITS) {
		ids.add("data");
	}
}

void add(CStringArray& ids, uint32_t family_id)
{
	switch(family_id) {
	case CYW43_FIRMWARE_FAMILY_ID:
		ids.add("cyw43-firmware");
		break;
	default:
		ids.add(String(family_id, HEX, 8));
		break;
	}
}

} // namespace UF2::Family
