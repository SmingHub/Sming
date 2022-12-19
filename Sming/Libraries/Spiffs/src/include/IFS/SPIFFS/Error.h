/**
 * Error.h
 * SPIFFS error codes
 *
 *  Created on: 31 Aug 2018
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the SPIFFS IFS Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <IFS/Types.h>

namespace IFS
{
namespace SPIFFS
{
inline bool isSpiffsError(int err)
{
	return err <= -10000;
}

int translateSpiffsError(int spiffs_error);

String spiffsErrorToStr(int err);
} // namespace SPIFFS
} // namespace IFS
