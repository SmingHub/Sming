#include "Client.h"

#include <Network/SSDP/Server.h>
#include <Network/UPnP/DeviceHost.h>

namespace Dial
{
DEFINE_FSTR(domain, "dial-multiscreen-org")
DEFINE_FSTR(service, "dial")

HttpClient Client::http;

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

	debug_d("Fetching description from URL: '%s'", location);
	Url url(location);
	auto request = new HttpRequest(url);
	request->setResponseStream(new LimitedMemoryStream(maxDescriptionSize));
	request->onRequestComplete(RequestCompletedDelegate(&Client::onDescription, this));
	http.send(request);
}

int Client::onDescription(HttpConnection& connection, bool success)
{
	if(!success) {
		debug_e("Fetch failed");
		return 0;
	}

	debug_i("Received description");
	auto response = connection.getResponse();
	if(response->stream == nullptr) {
		debug_e("No body");
		return 0;
	}

	String url = response->headers[_F("Application-URL")];
	if(url) {
		applicationUrl = url;
	}

	String content;
	response->stream->moveString(content);
	XML::Document doc;
	XML::deserialize(doc, content);

	descriptionUrl = connection.getRequest()->uri;

	debug_d("Found DIAL device with searchType: %s", String(searchType).c_str());
	if(onConnected) {
		onConnected(*this, doc, response->headers);
	}

	return 0;
}

bool Client::connect(Connected callback, const String& urn)
{
	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return false;
	}

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
	auto request = new HttpRequest(descriptionUrl);
	request->setResponseStream(new LimitedMemoryStream(maxDescriptionSize));
	request->onRequestComplete(RequestCompletedDelegate(&Client::onDescription, this));
	http.send(request);

	return true;
}

App& Client::getApp(const String& applicationId)
{
	auto app = apps[applicationId];
	if(!app) {
		app = new App(applicationId, applicationUrl);
	}

	return *app;
}

} // namespace Dial
