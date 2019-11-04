/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Url.cpp
 *
 * Code based on http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform
 *
 ****/

#include "Url.h"
#include "libyuarel/yuarel.h"
#include "WebHelpers/escape.h"
#include "Print.h"

/**
 * @brief Common URI scheme strings
 */
#define XX(name, str, port) DEFINE_FSTR(URI_SCHEME_##name, #str)
URI_SCHEME_MAP(XX)
#undef XX

// If no scheme is specified we'll default to http
#define URI_SCHEME_DEFAULT URI_SCHEME_HTTP

Url& Url::operator=(String urlString)
{
	struct yuarel url;
	if(yuarel_parse(&url, urlString.begin()) < 0) {
		debug_e("URL parsing failed: %s", urlString.c_str());
		return *this;
	}

	if(url.scheme == nullptr) {
		Scheme = URI_SCHEME_DEFAULT;
	} else {
		Scheme = url.scheme;
		Scheme.toLowerCase();
	}
	User = uri_unescape_inplace(url.username);
	Password = uri_unescape_inplace(url.password);
	Host = url.host;
	Port = url.port ?: getDefaultPort(Scheme);
	Path = String('/') + uri_unescape_inplace(url.path);
	Query.parseQuery(url.query);
	Fragment = uri_unescape_inplace(url.fragment);

	return *this;
}

int Url::getDefaultPort(const String& scheme)
{
#define XX(name, str, port)                                                                                            \
	if(scheme == URI_SCHEME_##name)                                                                                    \
		return port;                                                                                                   \
	else
	URI_SCHEME_MAP(XX)
#undef XX
	return 0;
}

String Url::getHostWithPort() const
{
	String result = Host;
	if(Port != 0) {
		result += ':';
		result += Port;
	}

	return result;
}

String Url::toString() const
{
	String result = Scheme;
	if(result.length() == 0) {
		result = URI_SCHEME_DEFAULT;
	}
	result += _F("://");
	if(User.length() != 0) {
		result += User;
		if(Password.length() != 0) {
			result += ':';
			result += Password;
		}
	}

	result += getHostWithPort();
	result += getPathWithQuery();

	if(Fragment) {
		result += '#';
		result += uri_escape(Fragment);
	}

	return result;
}

String Url::getPathWithQuery() const
{
	String result = (Path.length() == 0) ? String('/') : uri_escape(Path);
	result += Query;
	return result;
}

String Url::getFileName() const
{
	int sepIndex = Path.lastIndexOf('/');
	return (sepIndex < 0) ? Path : Path.substring(sepIndex + 1);
}

void Url::debugPrintTo(Print& p) const
{
	auto emit = [&](const char* tag, const String& value) {
		p.print(tag);
		p.print(_F(" = "));
		if(value) {
			p.print('[');
			p.print(value);
			p.print(']');
		} else {
			p.print(_F("(null)"));
		}
		p.println();
	};

	emit(_F("Scheme"), Scheme);
	emit(_F("User"), User);
	emit(_F("Password"), Password);
	emit(_F("Host"), Host);
	p.println(_F("Port = ") + String(Port));
	emit(_F("Path"), Path);

	p.println(_F("Query:"));
	Query.debugPrintTo(p);

	emit(_F("Fragment"), Fragment);
}
