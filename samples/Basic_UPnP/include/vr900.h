/*
 * These are the bones of a network gateway built from the XML reports of a TP-Link Archer VR900 VDSL2 device.
 * You can find these reports in data/vr900.
 *
 * This should give you some idea of how to manually translate existing devices into useable components from
 * which you can construct your own compliant devices.
 */

#include <Network/UPnP/RootDevice.h>

namespace VR900
{
using namespace UPnP;

class BaseDevice : public Device
{
public:
	static String getUrnBase()
	{
		// Use a different value to avoid conflict with existing device
		return F("uuid:8f0865b3-f5da-4ad5-85b7-7404637fdf");
	}

	static String getCommonField(Field desc)
	{
		switch(desc) {
		case Field::friendlyName:
		case Field::modelName:
			return F("Sample Network Gateway");
		case Field::manufacturer:
			return F("Sming");
		case Field::manufacturerURL:
			return F("https://github.com/SmingHub/Sming");
		case Field::modelDescription:
			return F("Sample Network Gateway Device");
		case Field::modelNumber:
		case Field::serialNumber:
			return F("1.0");
		default:
			return nullptr;
		}
	}

	String getField(Field desc) override
	{
		String s = getCommonField(desc);
		if(s) {
			return s;
		}

		switch(desc) {
		case Field::modelURL:
			return "/";
		default:
			return Device::getField(desc);
		}
	}
};

class WANIPConnection : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("WANIPConnection:1");
		case Field::serviceId:
			return F("urn:upnp-org:serviceId:WANIPConn1");
			//		case Field::controlURL:
			//		case Field::eventSubURL:
			//			return F("/upnp/control/WANIPConn1");
			//		case Field::SCPDURL:
			//			return F("/gateconnSCPD.xml");
		default:
			return Service::getField(desc);
		}
	}

	void handleAction(ActionInfo& info) override
	{
	}
};

class WanConnectionDevice : public BaseDevice
{
public:
	WanConnectionDevice()
	{
		addService(&connection);
	}

	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("WANConnectionDevice:1");
		case Field::friendlyName:
			return F("WAN Connection");
		case Field::UDN:
			return getUrnBase() + "39";
		default:
			return BaseDevice::getField(desc);
		}
	}

private:
	WANIPConnection connection;
};

class WANCommonInterfaceConfig : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("WANCommonInterfaceConfig:1");
		case Field::serviceId:
			return F("urn:upnp-org:serviceId:WANCommonIFC1");
			//		case Field::controlURL:
			//		case Field::eventSubURL:
			//			return F("/upnp/control/WANCommonIFC1");
			//		case Field::SCPDURL:
			//			return F("/gateicfgSCPD.xml");
		default:
			return Service::getField(desc);
		}
	}

	void handleAction(ActionInfo& info) override
	{
	}
};

class WanDevice : public BaseDevice
{
public:
	WanDevice()
	{
		addService(&interfaceConfig);
		addDevice(&connection1);
		addDevice(&connection2);
	}

	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("WANDevice:1");
		case Field::friendlyName:
			return F("WAN Device");
		case Field::UDN:
			return getUrnBase() + "38";
		default:
			return BaseDevice::getField(desc);
		}
	}

private:
	WANCommonInterfaceConfig interfaceConfig;
	WanConnectionDevice connection1;
	WanConnectionDevice connection2;
};

class Layer3ForwardingService : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("Layer3Forwarding:1");
		case Field::serviceId:
			return F("urn:upnp-org:serviceId:L3Forwarding1");
			//		case Field::controlURL:
			//		case Field::eventSubURL:
			//			return F("/upnp/control/dummy");
			//		case Field::SCPDURL:
			//			return F("/dummy.xml");
		default:
			return Service::getField(desc);
		}
	}

	void handleAction(ActionInfo& info) override
	{
	}
};

class GatewayDevice : public RootDevice
{
public:
	GatewayDevice()
	{
		addDevice(&wan);
		addService(&layer3ForwardingService);
	}

	String getField(Field desc) override
	{
		String s = BaseDevice::getCommonField(desc);
		if(s) {
			return s;
		}

		switch(desc) {
		case Field::type:
			return F("InternetGatewayDevice:1");
		case Field::UDN:
			return BaseDevice::getUrnBase() + "37";
		default:
			return RootDevice::getField(desc);
		}
	}

private:
	WanDevice wan;
	Layer3ForwardingService layer3ForwardingService;
};

} // namespace VR900
