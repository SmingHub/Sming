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
#include "../../Wiring/FILO.h"
#include "../WebConstants.h"
#include "../URL.h"

#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE  (8*1024)
#endif

/* Number of maximum tcp connections to be kept in the pool */
#ifndef HTTP_REQUEST_POOL_SIZE
#define HTTP_REQUEST_POOL_SIZE 20
#endif

#include "../http-parser/http_parser.h"

/**
 * WARNING: For the moment the name "SimpleConcurrentQueue" is very misleading.
 */
template<typename T, int rawSize>
class SimpleConcurrentQueue: public FIFO<T, rawSize> {
public:
	virtual const T& operator[](unsigned int) const { }
	virtual T& operator[](unsigned int) { }

	T peek() const
	{
	  if(!FIFO<T, rawSize>::numberOfElements) {
		  return NULL;
	  }

	  return FIFO<T, rawSize>::peek();
	}

	T dequeue()
	{
	  if(!FIFO<T, rawSize>::numberOfElements) {
		return NULL;
	  }

	  return FIFO<T, rawSize>::dequeue();
	}
};

typedef HashMap<String, String> HttpParams;
typedef HashMap<String, String> HttpHeaders;
typedef enum http_method HttpMethod;

#endif /* _SMING_CORE_HTTP_COMMON_H_ */
