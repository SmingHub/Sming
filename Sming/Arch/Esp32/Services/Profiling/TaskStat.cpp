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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <bitset>
#include <cstdlib>

namespace Profiling
{
struct TaskStat::Info {
#if CONFIG_FREERTOS_USE_TRACE_FACILITY && CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
	unsigned count{0};
	uint32_t runTime;
	TaskStatus_t status[maxTasks];
#endif
};

TaskStat::TaskStat(Print& out) : out(out)
{
#if CONFIG_FREERTOS_USE_TRACE_FACILITY && CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
	taskInfo.reset(new Info[2]);
#endif
}

TaskStat::~TaskStat()
{
}

bool TaskStat::update()
{
#if CONFIG_FREERTOS_USE_TRACE_FACILITY && CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS

	// Get current task states
	auto& info = taskInfo[endIndex];
	info.count = uxTaskGetSystemState(info.status, maxTasks, &info.runTime);
	if(info.count == 0) {
		out.println(_F("[TaskStat] uxTaskGetSystemState returned 0"));
		return false;
	}

	std::qsort(info.status, info.count, sizeof(TaskStatus_t), [](const void* a, const void* b) -> int {
		auto s1 = static_cast<const TaskStatus_t*>(a);
		auto s2 = static_cast<const TaskStatus_t*>(b);
		int res = 0;
#if CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
		res = s1->xCoreID - s2->xCoreID;
#endif
		if(res == 0) {
			res = s2->uxCurrentPriority - s1->uxCurrentPriority;
		}
		if(res == 0) {
			res = s1->xTaskNumber - s2->xTaskNumber;
		}
		return res;
	});

	if(startIndex == endIndex) {
		endIndex = 1;
		return true;
	}

	auto& startInfo = taskInfo[startIndex];
	auto& endInfo = taskInfo[endIndex];

	// Set indices for next update
	endIndex = startIndex;
	startIndex = 1 - endIndex;

	// Calculate totalElapsedTime in units of run time stats clock period.
	uint32_t totalElapsedTime = endInfo.runTime - startInfo.runTime;
	if(totalElapsedTime == 0) {
		out.println(_F("[TaskStat] Run time was 0: Have you set CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS?"));
		return false;
	}

	std::bitset<maxTasks> startMatched, endMatched;

	PSTR_ARRAY(hdrfmt, "#   | Core | Prio | Run Time | % Time | Name");
	PSTR_ARRAY(datfmt, "%-3u |   %c  | %4u | %8u |  %3u%%  | %s\r\n");
	out.println();
	out.println(hdrfmt);
	// Match each task in startInfo.status to those in the endInfo.status
	for(unsigned i = 0; i < startInfo.count; i++) {
		int k = -1;
		for(unsigned j = 0; j < endInfo.count; j++) {
			if(startInfo.status[i].xHandle == endInfo.status[j].xHandle) {
				k = j;
				startMatched[i] = endMatched[j] = true;
				break;
			}
		}
		// Check if matching task found
		if(k >= 0) {
			auto& status = startInfo.status[i];
			uint32_t taskElapsedTime = endInfo.status[k].ulRunTimeCounter - status.ulRunTimeCounter;
			uint32_t percentageTime = (taskElapsedTime * 100UL) / (totalElapsedTime * portNUM_PROCESSORS);
			BaseType_t coreId{CONFIG_FREERTOS_NO_AFFINITY};
#if CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
			coreId = status.xCoreID;
#endif
			out.printf(datfmt, status.xTaskNumber, (coreId == CONFIG_FREERTOS_NO_AFFINITY) ? '-' : ('0' + coreId),
					   status.uxCurrentPriority, taskElapsedTime, percentageTime, status.pcTaskName);
		}
	}

	// Print unmatched tasks
	for(unsigned i = 0; i < startInfo.count; i++) {
		if(!startMatched[i]) {
			out.printf("Deleted: %s\r\n", startInfo.status[i].pcTaskName);
		}
	}
	for(unsigned i = 0; i < endInfo.count; i++) {
		if(!endMatched[i]) {
			out.printf("Created: %s\r\n", endInfo.status[i].pcTaskName);
		}
	}

	return true;

#else
	out.println("[TaskStat] Tracing disabled");
	return false;
#endif
}

} // namespace Profiling
