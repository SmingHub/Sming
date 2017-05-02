/* ThingSpeak.h
 *
 *  Created on: 01-01-2017
 *  Author: Gustlik (Ogyb)
 *  \version 0.9
 */

#ifndef ThingSpeak_h
#define ThingSpeak_h

#include "../../SmingCore/SmingCore.h"

#ifndef TS_MAX_FIELD
#define TS_MAX_FIELD 8         	//how many fields write up max (max 8)
#endif

#define MAX_FIELD_LENGTH 64		//TS accept max 255

#define THINGSPEAK_URL "http://api.thingspeak.com/"		// Server URL -> "http://url.com/"
#define THINGSPEAK_PORT_NUMBER 80

#define OK_SUCCESS              200     //	OK / Success
#define ERR_BADAPIKEY           400     //	Incorrect API key
#define ERR_BADURL              404     //	Incorrect API key

#define ERR_FIELD_NUM_RANGE 	-101	//	Field number < 1 or Field > TS_MAX_FIELD
#define ERR_FIELD_TOLONG		-102	//	Field length > MAX_FIELD_LENGTH
#define ERR_NO_ITEM_SET			-201	//	No called "setField()" before use "writeFields()" or set empty value
#define ERR_IS_PROCESING		-202	//	HTTP query processing
#define ERR_HTTPCLIENT_SENDT	-300	//	Connection error

class ThingSpeak
{
public:
	ThingSpeak();
	ThingSpeak(String);
	virtual ~ThingSpeak();

	/** @brief  Set API key to writing data
	 *  @param  "APIKey" String od API key
	 *  @note   You can also set in the constructor.
	 */
	void setWriteAPIKey(String);

	/** @brief  Adding the value of a particular field.
	 *  @param  iField uint8_t - field id of field
	 *  @param  value - Value of field (int, float, ...)
	 *  @retval uint8_t - error or succas code
	 */
	template <typename T> uint8_t setField(uint8_t iField, T value){
		return setField(iField, String(value));
	}

	/** @brief  Adding the value of next field (last used field +1).
	 *  @param  value - Value of field (int, float, ...)
	 *  @retval uint8_t - error or succas code
	 *  @note   Use for addition in the order (field 1,2,3, ...)
	 */
	template <typename V> uint8_t setNextField(V value){
		if(lastId > TS_MAX_FIELD)
		{
			lastId = 1;
		}
		return setField(lastId, value);
	}

	/** @brief  Cleaning all field values.
	 */
	void cleanFields();

	/** @brief  Sending added field data to server and clean cache.
	 *  @retval uint8_t - error or succas code
	 *  @note   Use for getStstus() to get error or the entry ID of the update.
	 */
	int sendFields();

	/** @brief  Sending added field data to server and clean cache.
	 *  @retval uint8_t - error or succas code
	 *  @note   Use for getStstus() to get error or the entry ID of the update.
	 */
	int getStstus();

private:

	void onDataSent(HttpClient& , bool);
	HttpClient httpClient;
	String writeAPIKey;
	uint8_t lastId = 1;
	String fieldValue[TS_MAX_FIELD];
	int httpStatus = 0;

};
template <> uint8_t ThingSpeak::setField(uint8_t, String);
#endif
