#include "Client.h"
#include <Network/SSDP/Server.h>

namespace Panasonic
{
namespace VieraTV
{
#define XX(action, description) #action "\0"
DEFINE_FSTR_LOCAL(vieraCommands, VIERA_COMMAND_MAP(XX))
#undef XX

#define XX(id, name, code) #code "\0"
DEFINE_FSTR_LOCAL(vieraApps, VIERA_APP_MAP(XX))
#undef XX

String toString(enum Action a)
{
	return CStringArray(vieraCommands)[(int)a];
}

String toString(enum ApplicationId a)
{
	return CStringArray(vieraApps)[(int)a];
}

bool Client::sendCommand(Action action)
{
	Command cmd;
	cmd.type = Command::Type::REMOTE;
	cmd.name = F("X_SendKey");

	String text = F("<X_KeyEvent>NRC_");
	text += toString(action);
	text += F("-ONOFF</X_KeyEvent>");

	setParams(cmd, text);

	return sendRequest(cmd);
}

bool Client::switchToHdmi(size_t input)
{
	Command cmd;
	cmd.type = Command::Type::REMOTE;
	cmd.name = F("X_SendKey");
	String text = F("<X_KeyEvent>NRC_HDMI");
	text += (input - 1);
	text += F("-ONOFF</X_KeyEvent>");

	setParams(cmd, text);

	return sendRequest(cmd);
}

bool Client::launch(const String& applicationId)
{
	Command cmd;
	cmd.type = Command::Type::REMOTE;
	cmd.name = F("X_LaunchApp");

	String text =
		F("<X_AppType>vc_app</X_AppType><X_LaunchKeyword>product_id=") + applicationId + F("</X_LaunchKeyword>");

	setParams(cmd, text);

	return sendRequest(cmd);
}

bool Client::getVolume(GetVolumeCallback onVolume)
{
	RequestCompletedDelegate requestCallback = [this, onVolume](HttpConnection& connection, bool successful) -> int {
		/* @see: docs/RequestResponse.txt for sample communication */
		CStringArray path("s:Body");
		path.add("u:GetVolumeResponse");
		path.add("CurrentVolume");

		auto node = this->getNode(connection, path);
		if(node != nullptr) {
			onVolume((int)node->value());

			return true;
		}

		return false;
	};

	Command cmd;
	cmd.type = Command::Type::RENDER;
	cmd.name = "GetVolume";

	String text = "<InstanceID>0</InstanceID><Channel>Master</Channel>";

	setParams(cmd, text);

	return sendRequest(cmd, requestCallback);
}

bool Client::setVolume(size_t volume)
{
	if(volume > 100) {
		debug_e("Volume must be in range from 0 to 100");
		return false;
	}

	Command cmd;
	cmd.type = Command::Type::RENDER;
	cmd.name = "SetVolume";
	String text = "<InstanceID>0</InstanceID><Channel>Master</Channel><DesiredVolume>";
	text += volume;
	text += "</DesiredVolume>";

	setParams(cmd, text);

	return sendRequest(cmd);
}

bool Client::getMute(GetMuteCallback onMute)
{
	RequestCompletedDelegate requestCallback = [this, onMute](HttpConnection& connection, bool successful) -> int {
		/* @see: docs/RequestResponse.txt for sample communication */
		CStringArray path("s:Body");
		path.add("u:GetMuteResponse");
		path.add("CurrentMute");
		auto node = this->getNode(connection, path);
		if(node != nullptr) {
			onMute((bool)node->value());

			return true;
		}

		return false;
	};

	Command cmd;
	cmd.type = Command::Type::RENDER;
	cmd.name = "GetMute";

	String text = "<InstanceID>0</InstanceID><Channel>Master</Channel>";

	setParams(cmd, text);

	return sendRequest(cmd, requestCallback);
}

bool Client::setMute(bool enable)
{
	Command cmd;
	cmd.type = Command::Type::RENDER;
	cmd.name = "SetMute";

	String text = "<InstanceID>0</InstanceID><Channel>Master</Channel><DesiredMute>";
	text += (enable ? '1' : '0');
	text += "</DesiredMute>";

	setParams(cmd, text);

	return sendRequest(cmd);
}

bool Client::sendRequest(Command command, RequestCompletedDelegate requestCallack)
{
	String path = F("/nrc/control_0");
	String urn = F("panasonic-com:service:p00NetworkControl:1");
	if(command.type == Command::Type::RENDER) {
		path = F("/dmr/control_0");
		urn = F("schemas-upnp-org:service:RenderingControl:1");
	}

	actionTag = nullptr;

	if(!envelope.initialise()) {
		return false;
	}

	auto body = envelope.body();
	if(body == nullptr) {
		return false;
	}

	String tag = "u:" + command.name;
	actionTag = XML::appendNode(body, tag);
	XML::appendAttribute(actionTag, "xmlns:u", urn);

	if(command.params != nullptr) {
		auto doc = body->document();
		auto commandNode = doc->first_node("s:Envelope")->first_node("s:Body")->first_node(tag.c_str());
		for(XML::Node* child = command.params->first_node(); child; child = child->next_sibling()) {
			auto node = doc->clone_node(child);
			commandNode->append_node(node);
		}
	}

	const String content = XML::serialize(envelope.doc, false);

	debug_d("Content XML: %s\n", content.c_str());

	HttpHeaders headers;
	headers[HTTP_HEADER_CONTENT_TYPE] = F("text/xml; charset=\"utf-8\"");
	headers["SOAPACTION"] = "\"urn:" + urn + '#' + command.name + '"';

	HttpRequest* request = new HttpRequest;
	request->method = HTTP_POST;
	request->headers.setMultiple(headers);
	request->uri = getDescriptionUrl();
	request->uri.Path = path;
	request->setBody(content);

	if(requestCallack != nullptr) {
		request->onRequestComplete(requestCallack);
	}

	return http.send(request);
}

} // namespace VieraTV

} // namespace Panasonic
