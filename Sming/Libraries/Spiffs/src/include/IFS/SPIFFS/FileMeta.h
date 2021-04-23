/**
 * FileMeta.h
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

#pragma once

#include <IFS/Access.h>
#include <IFS/FileAttributes.h>
#include <IFS/Compression.h>

namespace IFS
{
namespace SPIFFS
{
/**
 * @brief Content of SPIFFS metadata area
 */
struct FileMeta {
	/// This number is made up, but serves to identify that metadata is valid
	static constexpr uint32_t Magic{0xE3457A77};

	// Magic
	uint32_t magic;
	// Modification time
	TimeStamp mtime;
	// FileAttributes - default indicates content has changed
	FileAttributes attr;
	// Security
	ACL acl;
	// Compression
	Compression compression;

	void init()
	{
		*this = FileMeta{};
		magic = Magic;
		mtime = fsGetTimeUTC();
		acl.readAccess = UserRole::Admin;
		acl.writeAccess = UserRole::Admin;
	}

	void* getAttributePtr(AttributeTag tag)
	{
		switch(tag) {
		case AttributeTag::ModifiedTime:
			return &mtime;
		case AttributeTag::Acl:
			return &acl;
		case AttributeTag::Compression:
			return &compression;
		case AttributeTag::FileAttributes:
			return &attr;
		default:
			return nullptr;
		}
	}
};

#define FILEMETA_SIZE 16
static_assert(sizeof(FileMeta) == FILEMETA_SIZE, "FileMeta wrong size");

#if SPIFFS_OBJ_META_LEN >= FILEMETA_SIZE
#define SPIFFS_STORE_META
#define SPIFFS_USER_METALEN (SPIFFS_OBJ_META_LEN - FILEMETA_SIZE)
#else
#define SPIFFS_USER_METALEN 0
#endif

struct SpiffsMetaBuffer {
	FileMeta meta;
	uint8_t user[SPIFFS_USER_METALEN];

	enum class Flag {
		dirty,
	};
	BitSet<uint8_t, Flag> flags;

	void init()
	{
		meta.init();
		memset(user, 0xFF, sizeof(user));
	}

	template <typename T> void assign(const T& data)
	{
		static_assert(sizeof(data) == offsetof(SpiffsMetaBuffer, flags), "SPIFFS metadata assign() size incorrect");
		memcpy(reinterpret_cast<void*>(this), data, sizeof(data));

		// If metadata uninitialised, then initialise it now
		if(meta.magic != FileMeta::Magic) {
			meta.init();
			flags += Flag::dirty;
		}
	}

	void copyTo(Stat& stat)
	{
		stat.acl = meta.acl;
		stat.attr = meta.attr;
		stat.mtime = meta.mtime;
		stat.compression = meta.compression;
	}

	void setFileTime(time_t t)
	{
		if(meta.mtime != t) {
			meta.mtime = t;
			flags += Flag::dirty;
		}
	}

	int getxattr(AttributeTag tag, void* buffer, size_t size)
	{
		if(tag >= AttributeTag::User) {
			return getUserAttribute(unsigned(tag) - unsigned(AttributeTag::User), buffer, size);
		}

		auto attrSize = getAttributeSize(tag);
		if(attrSize == 0) {
			return Error::BadParam;
		}
		if(size >= attrSize) {
			auto value = meta.getAttributePtr(tag);
			if(value != nullptr) {
				memcpy(buffer, value, attrSize);
			}
		}
		return attrSize;
	}

	int setxattr(AttributeTag tag, const void* data, size_t size)
	{
		if(tag >= AttributeTag::User) {
			return setUserAttribute(unsigned(tag) - unsigned(AttributeTag::User), data, size);
		}

		// Cannot delete standard attributes
		if(data == nullptr) {
			return Error::NotSupported;
		}
		if(size != getAttributeSize(tag)) {
			return Error::BadParam;
		}
		auto value = meta.getAttributePtr(tag);
		if(value == nullptr) {
			return Error::BadParam;
		}
		if(memcmp(value, data, size) == 0) {
			// No change
			return FS_OK;
		}
		memcpy(value, data, size);
		if(tag == AttributeTag::Compression) {
			meta.attr[FileAttribute::Compressed] = (meta.compression.type != Compression::Type::None);
		}
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
	 *   Unused space is set to 0xFF.
	 */
	int getUserAttribute(unsigned userTag, void* buffer, size_t size)
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

	int setUserAttribute(unsigned userTag, const void* data, size_t size)
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
};

} // namespace SPIFFS
} // namespace IFS
