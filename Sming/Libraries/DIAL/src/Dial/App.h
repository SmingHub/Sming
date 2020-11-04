/**
 * App.h
 *
 * Copyright 2020 slaff <slaff@attachix.com>
 *
 * This file is part of the Sming DIAL Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <SmingCore.h>

/** @defgroup   DIAL application
 *  @brief      Provides DIAL application control
 *  @ingroup    DIAL client
 *  @{
 */

namespace Dial
{
class App
{
public:
	using ResponseCallback = Delegate<void(App& app, HttpResponse& response)>;

	App(const String& name, const Url& appsUrl, size_t maxDescriptionSize = 2048)
		: name(name), maxDescriptionSize(maxDescriptionSize)
	{
		applicationUrl = Url(appsUrl.toString() + '/' + name);
		instanceUrl = applicationUrl;
	}

	String getName()
	{
		return name;
	}

	bool status(ResponseCallback onResponse);

	/**
	 *
	 */
	bool run(ResponseCallback onResponse = nullptr);

	bool run(const String& body, enum MimeType mime, ResponseCallback onResponse = nullptr);

	bool run(const HttpParams& params, ResponseCallback onResponse = nullptr);

	bool stop(ResponseCallback onResponse = nullptr);

private:
	String name;
	Url applicationUrl;
	String instanceUrl;
	size_t maxDescriptionSize;
	static HttpClient http;
};

} // namespace Dial

/** @} */
