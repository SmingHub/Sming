#include <Network/UPnP/RootDevice.h>

class TeaPot : public UPnP::RootDevice
{
public:
	TeaPot()
	{
	}

	String getField(Field desc) override
	{
		switch(desc) {
		case Field::deviceType:
			return F("upnp:tea-pot"); // This device is a tea pot
		case Field::UDN:
			return F("uuid:1231313131::upnp:tea-pot"); // This is the unique id of the device.
		case Field::friendlyName:
			return F("Sming Tea Pot");
		case Field::modelName:
			return F("Simple tea pot");
		case Field::manufacturer:
			return F("Sming");
		case Field::manufacturerURL:
			return F("https://github.com/SmingHub/Sming");
		case Field::modelDescription:
			return F("Simple UPnP test device for Sming");
		case Field::modelNumber:
			return F("1");
		case Field::serialNumber:
			return F("12345678");
		case Field::baseURL:
			return F("/teapot/"); // Serve up any files at this location
		default:
			return RootDevice::getField(desc);
		}
	}
};
