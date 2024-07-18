/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TaskStat.cpp
 *
 */

#include <Services/Profiling/TaskStat.h>

namespace Profiling
{
struct TaskStat::Info {
};

TaskStat::TaskStat(Print& out) : out(out)
{
}

TaskStat::~TaskStat() = default;

bool TaskStat::update()
{
	out.println("[TaskStat] Not Implemented");
	return false;
}

} // namespace Profiling
