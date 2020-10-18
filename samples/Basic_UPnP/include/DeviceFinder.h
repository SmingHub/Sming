#pragma once

#include <Network/UPnP/ControlPoint.h>
#include <Network/HttpClient.h>
#include <Data/Stream/LimitedMemoryStream.h>
#include <RapidXML.h>

/*
 * Helper class for handling DIAL searches.
 */
class DeviceFinder : public UPnP::ControlPoint, private HttpClient
{
public:
	bool formatMessage(SSDP::Message& msg, SSDP::MessageSpec& ms) override;

	void onNotify(SSDP::BasicMessage& msg) override;

private:
	int descriptionFetched(HttpConnection& conn, bool success);

private:
	CStringArray locations;
};
