#include "Client.h"

#include <Network/SSDP/Server.h>
#include <Network/UPnP/DeviceHost.h>

namespace Dial
{
HttpClient Client::http;

bool Client::formatMessage(SSDP::Message& message, SSDP::MessageSpec& ms)
{
	// Override the search target
	message["ST"] = searchType;
	return true;
}

void Client::onNotify(SSDP::BasicMessage& message)
{
	if(searchType != message["NT"] && searchType != message["ST"]) {
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

int Client::onDescription(HttpConnection& conn, bool success)
{
	if(!success) {
		debug_e("Fetch failed");
		return 0;
	}

	debug_i("Received description");
	auto response = conn.getResponse();
	if(response->stream == nullptr) {
		debug_e("No body");
		return 0;
	}

	if(response->headers.contains(_F("Application-URL"))) {
		applicationUrl = response->headers[_F("Application-URL")];
	}

	auto stream = reinterpret_cast<LimitedMemoryStream*>(response->stream);
	stream->print('\0');
	XML::Document doc;
	XML::deserialize(doc, stream->getStreamPointer());

	descriptionUrl = Url(conn.getRequest()->uri);

	debug_d("Found DIAL device with searchType: %s", searchType.c_str());
	if(onConnected) {
		onConnected(*this, doc, response->headers);
	}

	return 0;
}

bool Client::connect(ConnectedCallback callback, const String& type)
{
	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return false;
	}

	onConnected = callback;
	searchType = type;

	UPnP::deviceHost.registerControlPoint(this);

	auto message = new SSDP::MessageSpec(SSDP::MESSAGE_MSEARCH);
	message->object = this;
	message->repeat = 2;
	message->target = SSDP::TARGET_ROOT;
	SSDP::server.messageQueue.add(message, 0);

	return true;
}

bool Client::connect(const Url& descriptionUrl, ConnectedCallback callback)
{
	this->descriptionUrl = descriptionUrl;
	onConnected = callback;

	debug_d("Fetching '%s'", descriptionUrl.toString().c_str());
	auto request = new HttpRequest(descriptionUrl);
	request->setResponseStream(new LimitedMemoryStream(maxDescriptionSize));
	request->onRequestComplete(RequestCompletedDelegate(&Client::onDescription, this));
	http.send(request);

	return true;
}

App* Client::getApp(const String& applicationId)
{
	if(apps.contains(applicationId)) {
		return apps[applicationId];
	}

	apps[applicationId] = new App(applicationId, applicationUrl);

	return apps[applicationId];
}

XML::Node* Client::getNode(HttpConnection& connection, const CStringArray& path)
{
	HttpResponse* response = connection.getResponse();
	if(response->stream == nullptr) {
		debug_e("No body");
		return nullptr;
	}

	auto stream = reinterpret_cast<LimitedMemoryStream*>(response->stream);
	stream->print('\0');
	XML::Document doc;
	XML::deserialize(doc, stream->getStreamPointer());

	return getNode(doc, path);
}

XML::Node* Client::getNode(const XML::Document& doc, const CStringArray& path)
{
	auto node = doc.first_node();
	if(node != nullptr) {
		for(size_t i = 0; i < path.count(); i++) {
			node = node->first_node(path[i]);
			if(node == nullptr) {
				break;
			}
		}
	}

	return node;
}

} // namespace Dial
