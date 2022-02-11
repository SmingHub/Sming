#pragma once

#include <Print.h>
#include "Partition.h"

namespace Storage
{
namespace Debug
{
void printPartition(Print& out, Partition part, bool includeDevice = true);
void listPartitions(Print& out);
void listDevices(Print& out, bool fullPartitionInfo = true);

} // namespace Debug
} // namespace Storage
