#include <DeviceFinder.h>
#include <HardwareSerial.h>

DEFINE_FSTR(URN_multiscreen, "urn:dial-multiscreen-org:service:dial:1");

bool DeviceFinder::formatMessage(SSDP::Message& msg, SSDP::MessageSpec& ms)
{
	// Override the search target
	msg["ST"] = URN_multiscreen;
	return true;
}

void DeviceFinder::onNotify(SSDP::BasicMessage& msg)
{
	if(URN_multiscreen != msg["ST"]) {
		return;
	}

	auto loc = msg[HTTP_HEADER_LOCATION];
	if(locations.contains(loc)) {
		return; // Already found
	}
	locations += loc;

	Serial.println();
	Serial.println(F("SSDP message received:"));
	for(unsigned i = 0; i < msg.count(); ++i) {
		Serial.print(msg[i]);
	}
	Serial.println();

	debug_i("Fetching '%s'", loc);
	Url url(loc);
	auto req = new HttpRequest(url);
	req->setResponseStream(new LimitedMemoryStream(2048));
	req->onRequestComplete(RequestCompletedDelegate(&DeviceFinder::descriptionFetched, this));
	send(req);
}

int DeviceFinder::descriptionFetched(HttpConnection& conn, bool success)
{
	if(!success) {
		debug_e("Fetch failed");
		return 0;
	}

	debug_i("Received description");
	auto rsp = conn.getResponse();
	if(rsp->stream == nullptr) {
		debug_e("No body");
		return 0;
	}

	auto ms = reinterpret_cast<LimitedMemoryStream*>(rsp->stream);
	ms->print('\0');
	XML::Document doc;
	XML::deserialize(doc, ms->getStreamPointer());
	XML::Node* fn{nullptr};
	XML::Node* udn{nullptr};
	auto root = doc.first_node();
	if(root != nullptr) {
		auto dev = root->first_node("device");
		if(dev != nullptr) {
			fn = dev->first_node("friendlyName");
			udn = dev->first_node("UDN");
		}
	}

	auto value = [](XML::Node* node) -> const char* { return node ? node->value() : "not found"; };

	debug_i("friendlyName: %s, UDN: %s", value(fn), value(udn));

	for(unsigned i = 0; i < rsp->headers.count(); ++i) {
		Serial.print(rsp->headers[i]);
	}

	return 0;
}
