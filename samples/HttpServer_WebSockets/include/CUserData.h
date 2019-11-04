#ifndef C_USER_DATA_H_SAMPLE
#define C_USER_DATA_H_SAMPLE

#include <SmingCore.h>

//Simplified container modelling a user session
class CUserData
{
public:
	CUserData(const char* uName, const char* uData) : userName(uName), userData(uData)
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

#endif /*C_USER_DATA_H_SAMPLE*/
