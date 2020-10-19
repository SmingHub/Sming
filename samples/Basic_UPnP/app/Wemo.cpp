#include <Wemo.h>
#include <Platform/Station.h>

IMPORT_FSTR(WEMO_SERVICE_SCPD, COMPONENT_PATH "/config/wemo-service.xml");
IMPORT_FSTR(WEMO_METAINFO_SCPD, COMPONENT_PATH "/config/wemo-metainfo.xml");

namespace Wemo
{
String BasicEventService::getField(Field desc)
{
	switch(desc) {
	case Field::type:
		return F("basicevent:1");
	case Field::serviceId:
		return F("urn:Belkin:serviceId:basicevent1");
	default:
		return WemoService::getField(desc);
	}
}

void BasicEventService::handleAction(ActionInfo& info)
{
	auto act = info.actionName();
	if(act == "GetBinaryState") {
		if(info.createResponse()) {
			XML::appendNode(info.response, "BinaryState", device()->getState());
		}
		return;
	}

	if(act == "SetBinaryState") {
		bool state;
		if(info.getArgBool("BinaryState", state)) {
			debug_i("state = %u", state);
			device()->setState(state);
			info.createResponse();
		}
		return;
	}
}

String MetaInfoService::getField(Field desc)
{
	switch(desc) {
	case Field::type:
		return F("metainfo:1");
	case Field::serviceId:
		return F("urn:Belkin:serviceId:metainfo1");
	default:
		return WemoService::getField(desc);
	}
}

void MetaInfoService::handleAction(ActionInfo& info)
{
	//	auto res = createEnvelope(action);
	//	XML::appendNode(res, "BinaryState", 1);
	//	return res;
}

String Controllee::getField(Field desc)
{
	switch(desc) {
	case Field::domain:
		return F("Belkin");

	case Field::type:
		return F("controllee:1");

	case Field::friendlyName:
		return name_;

	case Field::manufacturer:
		return F("Belkin International Inc.");
	case Field::manufacturerURL:
		return F("http://www.belkin.com");
	case Field::modelDescription:
		return F("Belkin Plugin Socket 1.0");

	case Field::modelName:
		return F("Emulated Socket");
	case Field::modelNumber:
		return F("3.1415");
	case Field::modelURL:
		return F("http://www.belkin.com/plugin/");
	case Field::serialNumber: {
		String s = F("221517K01017xxxx");
		s[12] = hexchar((id_ >> 16) & 0x0f);
		s[13] = hexchar((id_ >> 8) & 0x0f);
		s[14] = hexchar((id_ >> 4) & 0x0f);
		s[15] = hexchar((id_ >> 0) & 0x0f);
		return s;
	}

	case Field::UDN: {
		String s;
		s += "uuid:Socket-1_0-";
		s += getField(Field::serialNumber);
		return s;
	}

	case Field::baseURL: {
		String url = RootDevice::getField(desc);
		url += _F("wemo/");
		url += id_;
		url += '/';
		return url;
	}

	case Field::serverId:
		return RootDevice::getField(desc) + " Unspecified"; //" Wemo/1.0";

	default:
		return RootDevice::getField(desc);
	}
}

bool Controllee::formatMessage(Message& msg, MessageSpec& ms)
{
	msg["01-NLS"] = F("b9200ebb-736d-4b93-bf03-835149d13983");
	msg["OPT"] = F("\"http://schemas.upnp.org/upnp/1/0/\"; ns=01");
	msg["X-User-Agent"] = F("redsonic");
	return RootDevice::formatMessage(msg, ms);
}

} // namespace Wemo
