/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpServerResource
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTP_COMMON_H_
#define _SMING_CORE_HTTP_COMMON_H_

#define ENABLE_HTTP_REQUEST_AUTH 1

#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Delegate.h"
#include "../WebConstants.h"
#include "../URL.h"
#include "../../Data/Structures.h"

#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE  (8*1024)
#endif

/* Number of maximum tcp connections to be kept in the pool */
#ifndef HTTP_REQUEST_POOL_SIZE
#define HTTP_REQUEST_POOL_SIZE 20
#endif

#include "../http-parser/http_parser.h"

typedef enum http_method HttpMethod;

enum HttpConnectionState
{
	eHCS_Ready = 0,
	eHCS_StartSending,
	eHCS_SendingHeaders,
	eHCS_StartBody,
	eHCS_SendingBody,
	eHCS_Sent
};


#endif /* _SMING_CORE_HTTP_COMMON_H_ */
