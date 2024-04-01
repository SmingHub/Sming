#include "CUserData.h"

void CUserData::addSession(WebsocketConnection& connection)
{
	activeWebSockets.addElement(&connection);
	connection.setUserData(this);
}

void CUserData::removeSession(WebsocketConnection& connection)
{
	if(activeWebSockets.removeElement(&connection)) {
		connection.setUserData(nullptr);
		Serial.println(F("Removed user session"));
	}
}

void CUserData::printMessage(WebsocketConnection& connection, const String& msg)
{
	int i = activeWebSockets.indexOf(&connection);
	if(i >= 0) {
		Serial << _F("Received msg on connection ") << i << ": " << msg << endl;
	}
}

void CUserData::logOut()
{
	for(auto skt : activeWebSockets) {
		skt->setUserData(nullptr);
	}

	activeWebSockets.removeAllElements();
}
