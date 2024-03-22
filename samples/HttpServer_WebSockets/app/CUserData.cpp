#include "CUserData.h"

//Simplified container modelling a user session

void CUserData::addSession(WebsocketConnection& connection)
{
	activeWebSockets.addElement(&connection);
	connection.setUserData(this);
}

void CUserData::removeSession(WebsocketConnection& connection)
{
	int i = activeWebSockets.indexOf(&connection);
	if(i < 0) {
		return;
	}

	activeWebSockets[i]->setUserData(nullptr);
	activeWebSockets.remove(i);
	Serial.println(F("Removed user session"));
}

void CUserData::printMessage(WebsocketConnection& connection, const String& msg)
{
	unsigned i = 0;
	for(; i < activeWebSockets.count(); i++) {
		if(connection == *(activeWebSockets[i])) {
			break;
		}
	}

	if(i < activeWebSockets.count()) {
		Serial << _F("Received msg on connection ") << i << ": " << msg;
	}
}

void CUserData::logOut()
{
	for(auto skt : activeWebSockets) {
		skt->setUserData(nullptr);
	}

	activeWebSockets.removeAllElements();
}
