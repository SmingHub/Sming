/**
 * flashmem.h - Flash memory emulation using backing file
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <hostlib/hostlib.h>

struct FlashmemConfig {
	const char* filename; ///< Path to flash backing file
	size_t createSize;	///< If file doesn't exist, created with this size
};

/**
 * @brief Open/create flash backing file
 * @param filename Path to flash backing file
 * @param createSize If file doesn't exist, created with this size
 * @retval bool true if backing file was opened successfully
 * @note Specify nullptr, 0 to use default values
 * All flash operations will be restricted to size of initial backing file
 */
bool host_flashmem_init(const FlashmemConfig& config);

void host_flashmem_cleanup();
