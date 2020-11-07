#include "Client.h"

#include <Network/SSDP/Server.h>
#include <Network/UPnP/DeviceHost.h>

namespace Dial
{
DEFINE_FSTR(domain, "dial-multiscreen-org")
DEFINE_FSTR(service, "dial")

void Client::onDescription(HttpConnection& connection, XML::Document& description, Connected callback)
{
	descriptionUrl = connection.getRequest()->uri;
	auto response = connection.getResponse();
	String url = response->headers[_F("Application-URL")];
	if(url) {
		applicationUrl = url;
	}

	debug_d("Found DIAL device with searchType: %s", toString(searchType).c_str());
	if(callback) {
		callback(*this, connection, description);
	}
}

bool Client::connect(Connected callback)
{
	UPnP::ServiceUrn urn(domain, service, version);
	return beginSearch(urn, [this, callback](HttpConnection& connection, XML::Document& description) {
		callback(*this, connection, description);
	});
}

bool Client::connect(const Url& descriptionUrl, Connected callback)
{
	debug_d("Fetching '%s'", descriptionUrl.toString().c_str());
	return requestDescription(descriptionUrl, [this, callback](HttpConnection& connection, XML::Document& description) {
		onDescription(connection, description, callback);
	});
}

App& Client::getApp(const String& applicationId)
{
	AppMap::Value app = apps[applicationId];
	if(!app) {
		app = new App(*this, applicationId, applicationUrl);
	}

	return *app;
}

} // namespace Dial
