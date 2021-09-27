/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TaskStat.h
 *
 */

#pragma once

#include <Print.h>
#include <memory>

namespace Profiling
{
/**
 * @brief Helper class to support printing of real-time task information
 * 
 * Code is refactored from the FreeRTOS Real Time Stats Example.
 * 
 * Requires these SDK configuration settings to be set:
 *
 * - FREERTOS_USE_TRACE_FACILITY
 * - FREERTOS_GENERATE_RUN_TIME_STATS
 * - FREERTOS_VTASKLIST_INCLUDE_COREID (optional)
 *
 */
class TaskStat
{
public:
	/**
	 * @brief Constructor
	 * @param out Where to print reports (e.g. Serial)
	 */
	TaskStat(Print& out);

	~TaskStat();

	/**
	 * @brief Update the report
	 *
	 * Nothing will be output the first time this is called.
	 * From then on, the stats will show the difference in task usage
	 * from the previous call.
	 */
	bool update();

private:
	Print& out;
	static constexpr size_t maxTasks{32};
	struct Info;
	std::unique_ptr<Info[]> taskInfo;
	uint8_t startIndex{0};
	uint8_t endIndex{0};
};

} // namespace Profiling
