/* ThingSpeak.h
 *
 *  Created on: 01-01-2017
 *  Author: Gustlik (Ogyb)
 */
#ifndef ThingSpeak_h
#define ThingSpeak_h

#include <SmingCore/SmingCore.h>
#ifndef TS_MAX_FIELD
#define TS_MAX_FIELD 8         	//how many fields write up max (max 8)
#endif

#define MAX_FIELD_LENGTH 64		//TS accept max 255

#define THINGSPEAK_URL "http://api.thingspeak.com/"		// Server URL -> "http://url.com/"
#define THINGSPEAK_IPADDRESS IPAddress(184,106,153,149)
#define THINGSPEAK_PORT_NUMBER 80

#define OK_SUCCESS              200     // OK / Success
#define ERR_BADAPIKEY           400     // Incorrect API key
#define ERR_BADURL              404     // Incorrect API key

#define ERR_FIELD_NUM_RANGE 	-101	//Field number < 1 or Field > TS_MAX_FIELD
#define ERR_FIELD_TOLONG		-102	//Field length > MAX_FIELD_LENGTH
#define ERR_NO_ITEM_SET			-201	//No called "setField()" before use "writeFields()" or set empty value
#define ERR_IS_PROCESING		-202	//HTTP query processing
#define ERR_HTTPCLIENT_SENDT	-300	//Connection error

class ThingSpeak
{
public:
	ThingSpeak();
	ThingSpeak(String);
	void Init(uint8_t);			//call for OneWire init
	virtual ~ThingSpeak();
	void setWriteAPIKey(String);
	int setField(uint8_t, String);
	void cleanFields();
	int writeFields();
	int getStstus();

private:

	void onDataSent(HttpClient& , bool);
	HttpClient httpClient;
	String writeAPIKey;
	String fieldValue[TS_MAX_FIELD];
	int httpStatus = 0;

};

#endif
