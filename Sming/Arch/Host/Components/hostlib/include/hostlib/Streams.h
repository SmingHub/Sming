/**
 * Streams.h - Print support for host output
 *
 * Copyright 2024 mikee47 <mike@sillyhouse.net>
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

#include <unistd.h>

namespace Host
{
class OutputStream : public Print
{
public:
	OutputStream(int fileno) : fileno(fileno)
	{
	}

	virtual size_t write(uint8_t c) override
	{
		return write(&c, 1);
	}

	size_t write(const uint8_t* buffer, size_t size) override
	{
		return ::write(fileno, buffer, size);
	}

private:
	int fileno;
};

OutputStream standardOutput(STDOUT_FILENO);
OutputStream standardError(STDERR_FILENO);

}; // namespace Host
