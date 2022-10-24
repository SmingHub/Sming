#include "include/Storage/Debug.h"
#include "include/Storage.h"
#include "include/Storage/SpiFlash.h"

namespace Storage
{
namespace Debug
{
void listPartitions(Print& out)
{
	out.println();
	out.println(_F("Registered partitions:"));
	for(auto part : Storage::findPartition()) {
		out << "- " << part << endl;
	}
	out.println();
}

void listPartitions(Print& out, const Device& device)
{
	out << device.getName() << _F(" partitions:") << endl;
	for(auto part : device.partitions()) {
		out << "- " << part << endl;
	}
	out.println();
}

void listDevices(Print& out, bool fullPartitionInfo)
{
	out.println();
	out.println(_F("Registered storage devices:"));
	for(auto& dev : Storage::getDevices()) {
		out << "  " << dev << _F(". Partitions:");
		if(!dev.partitions()) {
			out.println(_F(" None."));
			continue;
		}

		out.println();
		for(auto part : dev.partitions()) {
			if(fullPartitionInfo) {
				out << "    " << part << endl;
			} else {
				out << " " << part.name();
			}
		}
		out.println();
	}
	out.println();
}

} // namespace Debug
} // namespace Storage
