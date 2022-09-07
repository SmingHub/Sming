#include "include/Storage/Debug.h"
#include "include/Storage.h"
#include "include/Storage/SpiFlash.h"

namespace Storage
{
namespace Debug
{
void printPartition(Print& out, Partition part, bool includeDevice)
{
	out.print(part.name());
	if(includeDevice) {
		out.print(" on ");
		out.print(part.getDeviceName());
	}
	out.print(" (");
	out.print(part.typeString());
	out.print(_F(" @ 0x"));
	out.print(part.address(), HEX);
	out.print(_F(", size 0x"));
	out.print(part.size(), HEX);
	out.println(")");
}

void listPartitions(Print& out)
{
	out.println();
	out.println(_F("Registered partitions:"));
	for(auto part : Storage::findPartition()) {
		out.print("- ");
		printPartition(out, part);
	}
	out.println();
}

void listDevices(Print& out, bool fullPartitionInfo)
{
	out.println();
	out.println(_F("Registered storage devices:"));
	for(auto& dev : Storage::getDevices()) {
		out.print("  name = '");
		out.print(dev.getName());
		out.print(_F("', type = "));
		out.print(toString(dev.getType()));
		out.print(_F(", size = 0x"));
		out.print(dev.getSize(), HEX);
		out.print(_F(", partitions:"));
		if(dev.partitions().count() == 0) {
			out.println(_F(" None."));
			continue;
		}

		out.println();
		for(auto part : dev.partitions()) {
			if(fullPartitionInfo) {
				out.print("    ");
				printPartition(out, part, false);
			} else {
				out.print(" ");
				out.print(part.name());
			}
		}
		out.println();
	}
	out.println();
}

} // namespace Debug
} // namespace Storage
