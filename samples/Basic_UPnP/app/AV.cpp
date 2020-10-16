#include <AV.h>
#include <HardwareSerial.h>

IMPORT_FSTR(didl_template_xml, PROJECT_DIR "/data/didl-template.xml");

IMPORT_FSTR(contentDirectorySCPD, PROJECT_DIR "/data/vr900/8200/ContentDir.xml");
IMPORT_FSTR(connectionMgrSCPD, PROJECT_DIR "/data/vr900/8200/connectionMgr.xml");
IMPORT_FSTR(mediaReceiverRegistrarSCPD, PROJECT_DIR "/data/vr900/8200/X_MS_MediaReceiverRegistrar.xml");

#define DEFINE_ACTION_NAME(n) DEFINE_FSTR_LOCAL(act_##n, #n)
#define GET_ATTR(name) String a_##name = info.actionArg(F(#name))

namespace AV
{
#define IS_ACTION(name) info.actionIs(act_##name)

void MediaReceiverRegistrarService::handleAction(ActionInfo& info)
{
	if(info.createResponse()) {
		XML::appendNode(info.response, "Result", 1);
	}
}

void TransportService::handleAction(ActionInfo& info)
{
	// TODO
}

void ConnectionManagerService::handleAction(ActionInfo& info)
{
	DEFINE_ACTION_NAME(GetProtocolInfo);
	DEFINE_ACTION_NAME(GetCurrentConnectionIDs);
	DEFINE_ACTION_NAME(GetCurrentConnectionInfo);

	DEFINE_FSTR_LOCAL(protocolInfo,
					  "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM_ICO,http-get:*:image/"
					  "jpeg:DLNA.ORG_PN=JPEG_LRG_ICO,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN,http-get:*:image/"
					  "jpeg:DLNA.ORG_PN=JPEG_SM,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED,http-get:*:image/"
					  "jpeg:DLNA.ORG_PN=JPEG_LRG");

	if(IS_ACTION(GetProtocolInfo)) {
		if(info.createResponse()) {
			XML::appendNode(info.response, "Source", protocolInfo);
		}
		return;
	}

	if(IS_ACTION(GetCurrentConnectionIDs)) {
		if(info.createResponse()) {
			XML::appendNode(info.response, "ConnectionIDs", 0);
		}
		return;
	}

	if(IS_ACTION(GetCurrentConnectionInfo)) {
		if(info.createResponse()) {
			XML::appendNode(info.response, "RcsID", -1);
			XML::appendNode(info.response, "AVTransportID", -1);
			XML::appendNode(info.response, "PeerConnectionID", -1);
			XML::appendNode(info.response, "Direction", "Output");
			XML::appendNode(info.response, "Status", "OK");
		}
		return;
	}
}

/*
	<DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/"
		xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/"
		xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
		xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/">
		<container id="0" parentID="0" restricted="0" childCount="0" searchable="1">
			<upnp:searchClass includeDerived="1">object.item.audioItem</upnp:searchClass>
			<upnp:searchClass includeDerived="1">object.item.imageItem</upnp:searchClass>
			<upnp:searchClass includeDerived="1">object.item.videoItem</upnp:searchClass>
			<dc:title>stuff</dc:title>
			<upnp:class>object.container.storageFolder</upnp:class>
			<upnp:storageUsed>-1</upnp:storageUsed>
		</container>
	</DIDL-Lite>
*/
static XML::Node* createDIDL(XML::Document& doc)
{
	if(!XML::deserialize(doc, didl_template_xml)) {
		return nullptr;
	}
	auto didl = doc.first_node();
	assert(didl != nullptr);
	return didl;
}

static unsigned getObject(XML::Node* didl, int objectID)
{
	if(objectID > 1) {
		return 0;
	}

	auto isFolder = (objectID == 0);

	auto obj = XML::appendNode(didl, isFolder ? "container" : "item");

	XML::appendAttribute(obj, "id", objectID);
	XML::appendAttribute(obj, "parentID", objectID - 1);
	XML::appendAttribute(obj, "restricted", 1);

	if(isFolder) {
		XML::appendNode(obj, "dc:title", "Tunes");
		XML::appendNode(obj, "upnp:class", "object.container.storageFolder");
		//		auto sc = XML::appendNode(obj, "upnp:searchClass", "object.item.audioItem");
		//		XML::appendAttribute(sc, "includeDerived", 1);
		XML::appendAttribute(obj, "childCount", 1);
		//		XML::appendAttribute(obj, "searchable", 1);
		XML::appendNode(obj, "upnp:storageUsed", -1);
	} else {
		XML::appendNode(obj, "dc:title", "Track #1");
		XML::appendNode(obj, "upnp:class", "object.item.audioItem.musicTrack");

		auto res = XML::appendNode(obj, "res", F("http://192.168.1.254:8200/MediaItems/10947.flac"));
		XML::appendAttribute(res, "size", 1024);
		XML::appendAttribute(res, "duration", "0:00:10.000");
		XML::appendAttribute(res, "bitrate", 128000);
		XML::appendAttribute(res, "sampleFrequency", 44100);
		XML::appendAttribute(res, "nrAudioChannels", 1);
		XML::appendAttribute(
			res, "protocolInfo",
			F("http-get:*:audio/x-flac:DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000"));
	}

	return 1;
}

/*
REQUEST:

<u:GetSearchCapabilities xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1" />

RESPONSE:

<u:GetSearchCapabilitiesResponse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
	<SearchCaps>dc:creator,dc:date,dc:title,upnp:album,upnp:actor,upnp:artist,upnp:class,upnp:genre,@refID</SearchCaps>
</u:GetSearchCapabilitiesResponse>



      <u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
         <ObjectID>0</ObjectID>
         <BrowseFlag>BrowseMetadata</BrowseFlag>
         <Filter>*</Filter>
         <StartingIndex>0</StartingIndex>
         <RequestedCount>0</RequestedCount>
         <SortCriteria />
      </u:Browse>


		<u:BrowseResponse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
			<Result>&lt;DIDL-Lite xmlns:dc="http://purl.org/dc/elements/1.1/"
				xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/"
				xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
				xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/"&gt;
				&lt;container id="0" parentID="-1" restricted="1" childCount="4"
				searchable="1"&gt;&lt;upnp:searchClass
				includeDerived="1"&gt;object.item.audioItem&lt;/upnp:searchClass&gt;&lt;upnp:searchClass
				includeDerived="1"&gt;object.item.imageItem&lt;/upnp:searchClass&gt;&lt;upnp:searchClass
				includeDerived="1"&gt;object.item.videoItem&lt;/upnp:searchClass&gt;&lt;dc:title&gt;root&lt;/dc:title&gt;&lt;upnp:class&gt;object.container.storageFolder&lt;/upnp:class&gt;&lt;upnp:storageUsed&gt;-1&lt;/upnp:storageUsed&gt;&lt;/container&gt;&lt;/DIDL-Lite&gt;</Result>
			<NumberReturned>1</NumberReturned>
			<TotalMatches>1</TotalMatches>
			<UpdateID>6</UpdateID>
		</u:BrowseResponse>
*/
void ContentDirectoryService::handleAction(ActionInfo& info)
{
	DEFINE_FSTR_LOCAL(searchCapabilities,
					  "dc:creator,dc:date,dc:title,upnp:album,upnp:actor,upnp:artist,upnp:class,upnp:genre,@refID");
	DEFINE_FSTR_LOCAL(sortCapabilities, "dc:title,dc:date,upnp:class,upnp:album,upnp:originalTrackNumber");

	DEFINE_ACTION_NAME(GetSearchCapabilities);
	DEFINE_ACTION_NAME(GetSortCapabilities);
	DEFINE_ACTION_NAME(GetSystemUpdateID);
	DEFINE_ACTION_NAME(Browse);
	DEFINE_ACTION_NAME(Search);

	if(IS_ACTION(GetSearchCapabilities)) {
		if(info.createResponse()) {
			XML::appendNode(info.response, F("SearchCaps"), searchCapabilities);
		}

		return;
	}

	if(IS_ACTION(GetSortCapabilities)) {
		if(info.createResponse()) {
			XML::appendNode(info.response, F("SortCaps"), sortCapabilities);
		}

		return;
	}

	if(IS_ACTION(GetSystemUpdateID)) {
		if(info.createResponse()) {
			XML::appendNode(info.response, F("Id"), systemUpdateId);
		}

		return;
	}

	if(IS_ACTION(Browse)) {
		GET_ATTR(ObjectID);
		GET_ATTR(BrowseFlag);
		GET_ATTR(Filter);
		GET_ATTR(StartingIndex);
		GET_ATTR(RequestedCount);
		GET_ATTR(SortCriteria);

		if(info.createResponse()) {
			auto objectID = a_ObjectID.toInt();
			auto startingIndex = a_StartingIndex.toInt();
			unsigned count;
			XML::Document doc;
			auto didl = createDIDL(doc);
			if(a_BrowseFlag == "BrowseDirectChildren") {
				count = getObject(didl, objectID + 1 + startingIndex);
			} else {
				count = getObject(didl, objectID);
			}

			String data = XML::serialize(doc);
			XML::appendNode(info.response, "Result", data);
			XML::appendNode(info.response, "NumberReturned", count);
			XML::appendNode(info.response, "TotalMatches", 1);
			XML::appendNode(info.response, "UpdateID", systemUpdateId);
		}

		return;
	}

	if(IS_ACTION(Search)) {
		GET_ATTR(ContainerID);
		GET_ATTR(SearchCriteria);
		GET_ATTR(Filter);
		GET_ATTR(StartingIndex);
		GET_ATTR(RequestedCount);
		GET_ATTR(SortCriteria);

		if(info.createResponse()) {
			XML::appendNode(info.response, "Result");
			XML::appendNode(info.response, "NumberReturned", 0);
			XML::appendNode(info.response, "TotalMatches", 0);
			XML::appendNode(info.response, "UpdateID", systemUpdateId);
		}

		return;
	}
}

} // namespace AV
