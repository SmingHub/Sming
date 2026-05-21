#pragma once

#include <Data/CStringArray.h>

#define PARTITION_EXTRA_FAMILY_ID_MAX 3

namespace UF2::Family
{

void parseFlags(CStringArray& ids, uint32_t flags);
void add(CStringArray& ids, uint32_t family_id);

} // namespace UF2::Family
