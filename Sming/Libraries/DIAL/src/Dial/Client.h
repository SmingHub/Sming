#pragma once

#include <SmingCore.h>
#include <Data/CStringArray.h>
#include <Network/UPnP/ControlPoint.h>
#include "App.h"

using namespace rapidxml;

namespace Dial
{
class Client : public UPnP::ControlPoint
{
public:
	using ConnectedCallback = Delegate<void(Client&, const XML::Document& doc, const HttpHeaders& headers)>;

	Client(size_t maxDescriptionSize = 4096) : maxDescriptionSize(maxDescriptionSize)
	{
	}

	/**
	 * @brief Searches for a DIAL device identified by a search type
	 * @param callback will be called once such a device is auto-discovered
	 * @param urn unique identifier of the search type
	 *
	 * @retval true when the connect request can be started
	 */
	virtual bool connect(ConnectedCallback callback, const String& type = "urn:dial-multiscreen-org:service:dial:1");

	/**
	 * @brief Directly connects to a device's description xml URL.
	 * @param descriptionUrl the full URL where a description XML can be found.
	 * 		  For example: http://192.168.22.222:55000/nrc/ddd.xml";
	 * @param callback will be called once the XML is fetched
	 *
	 * @retval true when the connect request can be started
	 */
	bool connect(const Url& descriptionUrl, ConnectedCallback callback);

	bool formatMessage(SSDP::Message& msg, SSDP::MessageSpec& ms) override;

	void onNotify(SSDP::BasicMessage& msg) override;

	App* getApp(const String& applicationId);

	/**
	 * TODO: Move this method to XML::Document ...
	 *
	 * @brief Gets XML node by path
	 * @param doc the XML document
	 * @param path the paths that have to be traversed to get the node (excluding the root node).
	 *
	 * @retval node
	 *
	 */
	XML::Node* getNode(const XML::Document& doc, const CStringArray& path);

protected:
	static HttpClient http;

	Url getDescriptionUrl()
	{
		return descriptionUrl;
	}

	XML::Node* getNode(HttpConnection& connection, const CStringArray& path);

private:
	size_t maxDescriptionSize; // <<< Maximum size of TV XML description that is stored.
	ConnectedCallback onConnected;

	Url descriptionUrl;
	Url applicationUrl;
	String searchType;
	CStringArray uniqueServiceNames;
	ObjectMap<String, App> apps; // <<< list of invoked apps

	int onDescription(HttpConnection& conn, bool success);
};

} // namespace Dial
