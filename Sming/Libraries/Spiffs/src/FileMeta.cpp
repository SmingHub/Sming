/**
 * FileMeta.cpp
 *
 * Created on: 21 Jul 2018
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the SPIFFS IFS Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/IFS/SPIFFS/FileMeta.h"
#include <IFS/Error.h>

namespace IFS
{
namespace SPIFFS
{
void* FileMeta::getAttributePtr(AttributeTag tag)
{
	switch(tag) {
	case AttributeTag::ModifiedTime:
		return &mtime;
	case AttributeTag::ReadAce:
		return &acl.readAccess;
	case AttributeTag::WriteAce:
		return &acl.writeAccess;
	case AttributeTag::Compression:
		return &compression;
	case AttributeTag::FileAttributes:
		return &attr;
	default:
		return nullptr;
	}
}

int SpiffsMetaBuffer::enumxattr(AttributeEnumCallback callback, void* buffer, size_t bufsize)
{
	size_t count{0};
	AttributeEnum e{buffer, bufsize};

	for(unsigned i = 0; i < unsigned(AttributeTag::User); ++i) {
		auto tag = AttributeTag(i);
		if(isDefaultValue(tag)) {
			continue;
		}
		auto value = meta.getAttributePtr(tag);
		if(value == nullptr) {
			continue;
		}
		e.set(tag, value, getAttributeSize(tag));
		++count;
		if(!callback(e)) {
			return count;
		}
	}

#if SPIFFS_USER_METALEN
	for(unsigned i = 0; i < SPIFFS_USER_METALEN;) {
		uint8_t tagIndex = user[i++];
		uint8_t tagSize = user[i++];
		if(tagIndex == 0xff && tagSize == 0xff) {
			break;
		}
		e.set(AttributeTag(unsigned(AttributeTag::User) + tagIndex), &user[i], tagSize);
		++count;
		if(!callback(e)) {
			break;
		}
		i += tagSize;
	}
#endif

	return count;
}

int SpiffsMetaBuffer::getxattr(AttributeTag tag, void* buffer, size_t size)
{
	if(tag >= AttributeTag::User) {
		return getUserAttribute(unsigned(tag) - unsigned(AttributeTag::User), buffer, size);
	}

	auto value = meta.getAttributePtr(tag);
	if(value == nullptr || isDefaultValue(tag)) {
		return Error::NotFound;
	}

	auto attrSize = getAttributeSize(tag);
	memcpy(buffer, value, std::min(size, attrSize));
	return attrSize;
}

int SpiffsMetaBuffer::setxattr(AttributeTag tag, const void* data, size_t size)
{
	if(tag >= AttributeTag::User) {
		return setUserAttribute(unsigned(tag) - unsigned(AttributeTag::User), data, size);
	}

	// Cannot delete standard attributes
	if(data == nullptr) {
		return Error::NotSupported;
	}
	auto value = meta.getAttributePtr(tag);
	if(value == nullptr) {
		return Error::NotSupported;
	}
	if(size != getAttributeSize(tag)) {
		return Error::BadParam;
	}
	if(memcmp(value, data, size) == 0) {
		// No change
		return FS_OK;
	}
	memcpy(value, data, size);
	flags += Flag::dirty;
	return FS_OK;
}

/*
 * User tags are laid out in spare space as follows:
 *
 * 	uint8_t tag;
 *  uint8_t len;
 *  uint8_t data[];
 *
 * Unused space is set to 0xFF.
 */
int SpiffsMetaBuffer::getUserAttribute(unsigned userTag, void* buffer, size_t size)
{
#if SPIFFS_USER_METALEN
	if(userTag > 255) {
		return Error::BadParam;
	}

	for(unsigned i = 0; i < SPIFFS_USER_METALEN;) {
		uint8_t tagIndex = user[i++];
		uint8_t tagSize = user[i++];
		if(tagIndex != userTag) {
			i += tagSize;
			continue;
		}
		if(size > tagSize) {
			size = tagSize;
		}
		if(buffer != nullptr) {
			memcpy(buffer, &user[i], std::min(size_t(tagSize), size));
		}
		return tagSize;
	}
#else
	(void)userTag;
	(void)buffer;
	(void)size;
#endif

	return Error::NotFound;
}

int SpiffsMetaBuffer::setUserAttribute(unsigned userTag, const void* data, size_t size)
{
#if SPIFFS_USER_METALEN
	if(userTag > 255) {
		return Error::BadParam;
	}

	bool deleteFlag = (data == nullptr);

	for(unsigned i = 0; i < SPIFFS_USER_METALEN;) {
		uint8_t tagIndex = user[i++];
		uint8_t tagSize = user[i++];
		if(tagIndex == userTag) {
			// Found the tag - compare with new data
			if(!deleteFlag && tagSize == size && memcmp(data, &user[i], tagSize) == 0) {
				// No change
				return FS_OK;
			}
			// Remove the tag
			i -= 2;
			tagSize += 2;
			// Shift items above down
			memmove(&user[i], &user[i + tagSize], SPIFFS_USER_METALEN - i - tagSize);
			// Clear unused space to 0xFF
			memset(&user[SPIFFS_USER_METALEN - tagSize], 0xff, tagSize);
			flags += Flag::dirty;

			if(deleteFlag) {
				return FS_OK;
			}
			continue;
		}

		// End of list?
		if(tagIndex == 0xff && tagSize == 0xff) {
			// Room for new tag?
			if(i + size > SPIFFS_USER_METALEN) {
				break;
			}
			user[i - 2] = userTag;
			user[i - 1] = size;
			memcpy(&user[i], data, size);
			flags += Flag::dirty;
			return size;
		}

		i += tagSize;
	}
#else
	(void)userTag;
	(void)data;
	(void)size;
#endif

	// No room for attribute
	return Error::BufferTooSmall;
}

} // namespace SPIFFS
} // namespace IFS
