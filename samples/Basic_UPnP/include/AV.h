#include <Network/UPnP/RootDevice.h>
#include <Data/Stream/FlashMemoryStream.h>

DECLARE_FSTR(contentDirectorySCPD);
DECLARE_FSTR(connectionMgrSCPD);
DECLARE_FSTR(mediaReceiverRegistrarSCPD);

namespace AV
{
using namespace UPnP;

class MediaReceiverRegistrarService : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::domain:
			return F("microsoft.com");
		case Field::type:
			return F("X_MS_MediaReceiverRegistrar:1");
		case Field::serviceId:
			return F("urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar");
		default:
			return Service::getField(desc);
		}
	}

	IDataSourceStream* createDescription() override
	{
		return new FlashMemoryStream(mediaReceiverRegistrarSCPD);
	}

	void handleAction(ActionInfo& info) override;
};

class TransportService : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("AVTransport:1");
		case Field::serviceId:
			return F("urn:upnp-org:serviceId:AVTransport");
		default:
			return Service::getField(desc);
		}
	}

	void handleAction(ActionInfo& info) override;
};

class ConnectionManagerService : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("ConnectionManager:1");
		case Field::serviceId:
			return F("urn:upnp-org:serviceId:ConnectionManager");
		default:
			return Service::getField(desc);
		}
	}

	IDataSourceStream* createDescription() override
	{
		return new FlashMemoryStream(connectionMgrSCPD);
	}

	void handleAction(ActionInfo& info) override;
};

class ContentDirectoryService : public Service
{
public:
	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("ContentDirectory:1");
		case Field::serviceId:
			return F("urn:upnp-org:serviceId:ContentDirectory");
		default:
			return Service::getField(desc);
		}
	}

	IDataSourceStream* createDescription() override
	{
		return new FlashMemoryStream(contentDirectorySCPD);
	}

	void handleAction(ActionInfo& info) override;

private:
	int systemUpdateId = 1;
};

class Server : public RootDevice
{
public:
	Server()
	{
		//		addService(&transport);
		addService(&registrar);
		addService(&connection);
		addService(&contentDirectory);
	}

	String getField(Field desc) override
	{
		switch(desc) {
		case Field::type:
			return F("MediaServer:1");
		case Field::UDN:
			return F("uuid:440c3865-0cd9-4f43-950b-aab28b02d0c7");
		case Field::friendlyName:
			return F("Sming AV Server");
		case Field::modelName:
			return F("Windows Media Connect compatible (MiniDLNA)");
		case Field::manufacturer:
			return F("Sillyhouse Technical Solutions");
		case Field::manufacturerURL:
			return F("http://www.sillyhouse.net");
		case Field::modelDescription:
			return F("Test device for UPnP Audio");
			//		case Field::modelURL:
		case Field::modelNumber:
			return F("1");
		case Field::serialNumber:
			return F("12345678");
		case Field::serverId:
			return RootDevice::getField(desc) + F(" DLNADOC/1.50 MiniDLNA/1.1.0");
		default:
			return RootDevice::getField(desc);
		}
	}

	XML::Node* getDescription(XML::Document& doc, DescType descType) override
	{
		auto dev = RootDevice::getDescription(doc, descType);
		if(descType == DescType::content) {
			XML::appendNode(dev, _F("dlna:X_DLNADOC"), _F("DMS-1.50"));
			XML::appendAttribute(dev, _F("xmlns:dlna"), _F("urn:schemas-dlna-org:device-1-0"));
		}
		return dev;
	}

private:
	MediaReceiverRegistrarService registrar;
	TransportService transport;
	ConnectionManagerService connection;
	ContentDirectoryService contentDirectory;
};

} // namespace AV
