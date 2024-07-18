#pragma once

#include <SmingCore.h>

// Simplified container modelling a user session
class CUserData
{
public:
	CUserData()
	{
	}

	CUserData(const String& uName, const String& uData) : userName(uName), userData(uData)
	{
	}

	~CUserData()
	{
		logOut();
	}

	void addSession(WebsocketConnection& connection);
	void removeSession(WebsocketConnection& connection);
	void printMessage(WebsocketConnection& connection, const String& msg);
	void logOut();

private:
	String userName;
	String userData;
	Vector<WebsocketConnection*> activeWebSockets;
};
