/****`
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpResourceTree.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_RESOURCE_TREE_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_RESOURCE_TREE_H_

#include "HttpResource.h"

/** @brief Class to map URL paths to classes which handle them
 *  @note Use `set()` method to safely set a value
 */
class ResourceTree : public ObjectMap<String, HttpResource>
{
public:
	/** @brief The default resource handler, identified by "*" wildcard */
	void setDefault(HttpResource* resource)
	{
		set(String('*'), resource);
	}

	HttpResource* getDefault()
	{
		return find(String('*'));
	}

private:
	/* Potentially dangerous as `tree[path] = newValue` would overwrite existing value without freeing it */
	HttpResource*& operator[](const String& key);
};

#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_RESOURCE_TREE_H_ */
