#pragma once

#include <SmingCore.h>

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
