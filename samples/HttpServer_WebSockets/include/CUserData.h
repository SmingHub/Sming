#ifndef C_USER_DATA_H_SAMPLE
#define C_USER_DATA_H_SAMPLE

#include <user_config.h>
#include <SmingCore/SmingCore.h>

//Simplified container modelling a user session
class CUserData
{
public:
	CUserData(const char* uName, const char* uData);
	~CUserData();
	void addSession(WebSocketConnection& connection);
	void removeSession(WebSocketConnection& connection);
	void printMessage(WebSocketConnection& connection,const String &msg);
	void logOut();
private:
	String userName;
	String userData;
	Vector<WebSocketConnection*> activeWebSockets;
};

#endif /*C_USER_DATA_H_SAMPLE*/
