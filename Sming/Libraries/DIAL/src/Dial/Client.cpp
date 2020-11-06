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

	if(requestDescription(location)) {
		// Request queued
		// TODO: Consider what happens if request fails to complete
		uniqueServiceNames += uniqueServiceName;
	}
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

	debug_d("Found DIAL device with searchType: %s", toString(searchType).c_str());
	if(onConnected) {
		onConnected(*this, connection, description);
	}
}

bool Client::isConnecting()
{
	// TODO: Consider timeout and/or checking if HTTP request is in progress

	//	if(onConnected) {
	//		debug_e("Dial: Connection already in progress");
	//		return true;
	//	}

	return false;
}

bool Client::connect(Connected callback)
{
	if(isConnecting()) {
		return false;
	}

	UPnP::deviceHost.registerControlPoint(this);

	onConnected = callback;

	auto message = new SSDP::MessageSpec(SSDP::MessageType::msearch, SSDP::SearchTarget::root, this);
	message->setRepeat(2);
	SSDP::server.messageQueue.add(message, 0);

	return true;
}

bool Client::connect(const UPnP::ServiceUrn& urn, Connected callback)
{
	if(isConnecting()) {
		return false;
	}

	searchType = urn;
	return connect(callback);
}

bool Client::connect(const Url& descriptionUrl, Connected callback)
{
	if(isConnecting()) {
		return false;
	}

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
