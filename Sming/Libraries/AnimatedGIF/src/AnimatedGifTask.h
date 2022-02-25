/****
 * AnimatedGifTask.h
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
 * @author: Feb 2022 - Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#pragma once

#include <Task.h>
#include <AnimatedGIF.h>
#include <Graphics/Surface.h>

class AnimatedGifTask : public Task
{
public:
	AnimatedGifTask(Graphics::Surface& surface, const void* data, size_t length, bool inFlash);

	AnimatedGifTask(Graphics::Surface& surface, const FSTR::ObjectBase& data)
		: AnimatedGifTask(surface, data.data(), data.length(), true)
	{
	}

	~AnimatedGifTask()
	{
		gif.close();
	}

protected:
	void loop() override;

private:
	AnimatedGIF gif;
	Graphics::Surface& surface;
};
