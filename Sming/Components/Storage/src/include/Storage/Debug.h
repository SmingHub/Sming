#pragma once

#include <Print.h>
#include "Partition.h"

namespace Storage::Debug
{
void listPartitions(Print& out);
void listPartitions(Print& out, const Device& device);
void listDevices(Print& out, bool fullPartitionInfo = true);

} // namespace Storage::Debug
