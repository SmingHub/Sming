#include "CUserData.h"

//Simplified container modelling a user session
CUserData::CUserData(const char* uName, const char* uData) : userName(uName), userData(uData)
{
}
CUserData::~CUserData()
{
	logOut();
}

void CUserData::addSession(WebsocketConnection& connection)
{
	activeWebSockets.addElement(&connection);
	connection.setUserData((void*)this);
}

void CUserData::removeSession(WebsocketConnection& connection)
{
	for(int i = 0; i < activeWebSockets.count(); i++) {
		if(&connection == activeWebSockets[i]) {
			activeWebSockets[i]->setUserData(nullptr);
			activeWebSockets.remove(i);
			Serial.println("Removed user session");
			return;
		}
	}
}

void CUserData::printMessage(WebsocketConnection& connection, const String& msg)
{
	int i = 0;
	for(; i < activeWebSockets.count(); i++) {
		if(&connection == activeWebSockets[i]) {
			break;
		}
	}

	if(i < activeWebSockets.count()) {
		Serial.print("Received msg on connection ");
		Serial.print(i);
		Serial.print(" :");
		Serial.print(msg);
	}
}

void CUserData::logOut()
{
	for(int i = 0; i < activeWebSockets.count(); i++) {
		activeWebSockets[i]->setUserData(nullptr);
	}

	activeWebSockets.removeAllElements();
}
