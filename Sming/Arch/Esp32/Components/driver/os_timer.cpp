// Copyright 2010-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * ets_timer module implements a set of legacy timer APIs which are
 * used by the WiFi driver. This is done on top of the newer esp_timer APIs.
 * Applications should not use ets_timer functions, as they may change without
 * notice.
 */

#include <driver/os_timer.h>
#include <esp_timer.h>
#include <Platform/Clocks.h>
#include <esp_attr.h>

static bool isTimerValid(smg_timer_t* ptimer)
{
	return ptimer != nullptr && ptimer->handle != nullptr;
}

void smg_timer_setfn(smg_timer_t* ptimer, os_timer_func_t* pfunction, void* parg)
{
	assert(ptimer != nullptr);

	if(ptimer == nullptr) {
		return;
	}
	if(ptimer->handle != nullptr) {
		smg_timer_done(ptimer);
	}

	const esp_timer_create_args_t create_args = {
		.callback = [](void* arg) -> void {
			auto t = static_cast<smg_timer_t*>(arg);
			System.queueCallback(t->timer_func, t->timer_arg);
		},
		.arg = ptimer,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "smg_timer_t",
	};

	ESP_ERROR_CHECK(esp_timer_create(&create_args, &ptimer->handle));

	ptimer->timer_func = pfunction;
	ptimer->timer_arg = parg;
}

void IRAM_ATTR smg_timer_arm_ticks(smg_timer_t* ptimer, uint32_t ticks, bool repeat_flag)
{
	uint32_t us = OsTimerClock::ticksToTime<NanoTime::Microseconds>(ticks);
	smg_timer_arm_us(ptimer, us, repeat_flag);
}

void IRAM_ATTR smg_timer_arm_us(smg_timer_t* ptimer, uint32_t time_us, bool repeat_flag)
{
	assert(isTimerValid(ptimer));

	esp_timer_stop(ptimer->handle); // no error check
	if(repeat_flag) {
		ESP_ERROR_CHECK(esp_timer_start_periodic(ptimer->handle, time_us));
	} else {
		ESP_ERROR_CHECK(esp_timer_start_once(ptimer->handle, time_us));
	}
}

void IRAM_ATTR smg_timer_arm(smg_timer_t* ptimer, uint32_t time_ms, bool repeat_flag)
{
	assert(isTimerValid(ptimer));

	uint64_t time_us = 1000LL * uint64_t(time_ms);
	esp_timer_stop(ptimer->handle); // no error check
	if(repeat_flag) {
		ESP_ERROR_CHECK(esp_timer_start_periodic(ptimer->handle, time_us));
	} else {
		ESP_ERROR_CHECK(esp_timer_start_once(ptimer->handle, time_us));
	}
}

void smg_timer_done(smg_timer_t* ptimer)
{
	if(!isTimerValid(ptimer)) {
		return;
	}

	esp_timer_delete(ptimer->handle);
	*ptimer = smg_timer_t{};
}

void IRAM_ATTR smg_timer_disarm(smg_timer_t* ptimer)
{
	if(!isTimerValid(ptimer)) {
		return;
	}

	esp_timer_stop(ptimer->handle);
}
