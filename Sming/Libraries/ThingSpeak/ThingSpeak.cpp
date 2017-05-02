/* ThingSpeak.cpp
 *
 *  Created on: 01-01-2017
 *  Author: Gustlik (Ogyb)
 * \version 0.9
 */

#include "ThingSpeak.h"

ThingSpeak::ThingSpeak()
{
	cleanFields();
}

ThingSpeak::ThingSpeak(String APIKey)
{
	writeAPIKey = APIKey;
}

ThingSpeak::~ThingSpeak()
{
}

void ThingSpeak::setWriteAPIKey(String APIKey)
{
	writeAPIKey = APIKey;
}

//Used only for string value! Else data type use template (see .h file)
template <> uint8_t ThingSpeak::setField(uint8_t iField, String value)
{
	if(iField < 1 || iField > TS_MAX_FIELD) return ERR_FIELD_NUM_RANGE;
	if(value.length() < MAX_FIELD_LENGTH)
	{
		fieldValue[iField -1] = value;
	}
	lastId = iField + 1;
	return OK_SUCCESS;
}


int ThingSpeak::sendFields()
{
	bool noItem = true;
	String postMessage = THINGSPEAK_URL;
	postMessage += "update?key=" + writeAPIKey;
	for(uint8_t iField = 0; iField < 8; iField++)
	{
		if(fieldValue[iField].length() > 0)
		{
			postMessage += "&field" + String(iField +1) + "=" + fieldValue[iField];
			fieldValue[iField] = "";
			noItem = false;
		}
	}
	if(noItem)
	{
		return ERR_NO_ITEM_SET;
	}
	if(httpClient.isProcessing())
	{
		return ERR_IS_PROCESING;
	}
	httpClient.downloadString(postMessage, HttpClientCompletedDelegate(&ThingSpeak::onDataSent, this));
	return OK_SUCCESS;
}

void ThingSpeak::onDataSent(HttpClient& client, bool successful)
{
	httpStatus = ERR_HTTPCLIENT_SENDT;
	String response = client.getResponseString();
	if (response.length() > 0)
	{
		httpStatus = response.toInt();
	}
}

int ThingSpeak::getStstus()
{
	if(httpClient.isProcessing())
	{
		return ERR_IS_PROCESING;
	}
	return httpStatus;
}

void ThingSpeak::cleanFields()
{
	for(uint8_t iField = 0; iField < TS_MAX_FIELD; iField++)
	{
		fieldValue[iField] = "";
	}
}

