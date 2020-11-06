#include "Client.h"

#include <Network/SSDP/Server.h>
#include <Network/UPnP/DeviceHost.h>

namespace Dial
{
DEFINE_FSTR(domain, "dial-multiscreen-org")
DEFINE_FSTR(service, "dial")

String Client::getSearchType() const
{
	return searchType ?: UPnP::ServiceUrn(domain, service, version);
}

bool Client::formatMessage(SSDP::Message& message, SSDP::MessageSpec& ms)
{
	// Override the search target
	message["ST"] = getSearchType();
	return true;
}

void Client::onNotify(SSDP::BasicMessage& message)
{
	auto st = getSearchType();
	if(st != message["NT"] && st != message["ST"]) {
		return;
	}

	auto location = message[HTTP_HEADER_LOCATION];
	if(location == nullptr) {
		debug_d("No valid Location header found.");
		return;
	}

	auto uniqueServiceName = message["USN"];
	if(uniqueServiceName == nullptr) {
		debug_d("No valid USN header found.");
		return;
	}

	if(uniqueServiceNames.contains(uniqueServiceName)) {
		return; // Already found
	}
	uniqueServiceNames += uniqueServiceName;

	requestDescription(location);
}

bool Client::requestDescription(const String& url)
{
	return ControlPoint::requestDescription(url, DescriptionCallback(&Client::onDescription, this));
}

void Client::onDescription(HttpConnection& connection, XML::Document& description)
{
	descriptionUrl = connection.getRequest()->uri;
	auto response = connection.getResponse();
	String url = response->headers[_F("Application-URL")];
	if(url) {
		applicationUrl = url;
	}

	debug_d("Found DIAL device with searchType: %s", searchType.c_str());
	if(onConnected) {
		onConnected(*this, description, response->headers);
	}
}

bool Client::connect(Connected callback, const String& urn)
{
	onConnected = callback;
	searchType = urn;

	UPnP::deviceHost.registerControlPoint(this);

	auto message = new SSDP::MessageSpec(SSDP::MessageType::msearch, SSDP::SearchTarget::root, this);
	message->setRepeat(2);
	SSDP::server.messageQueue.add(message, 0);

	return true;
}

bool Client::connect(const Url& descriptionUrl, Connected callback)
{
	onConnected = callback;

	debug_d("Fetching '%s'", descriptionUrl.toString().c_str());
	return requestDescription(descriptionUrl);
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
