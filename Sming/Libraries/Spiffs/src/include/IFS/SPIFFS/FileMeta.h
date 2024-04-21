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

#include <IFS/Attribute.h>

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

	void* getAttributePtr(AttributeTag tag);
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

	void setFileTime(time_t t)
	{
		if(meta.mtime != t) {
			meta.mtime = t;
			flags += Flag::dirty;
		}
	}

	bool isDefaultValue(AttributeTag tag)
	{
		switch(tag) {
		case AttributeTag::FileAttributes:
			return meta.attr.none();
		case AttributeTag::Compression:
			return meta.compression.type == Compression::Type::None;
		default:
			return false;
		}
	}

	int enumxattr(const AttributeEnumCallback& callback, void* buffer, size_t bufsize);
	int getxattr(AttributeTag tag, void* buffer, size_t size);
	int setxattr(AttributeTag tag, const void* data, size_t size);
	int getUserAttribute(unsigned userTag, void* buffer, size_t size);
	int setUserAttribute(unsigned userTag, const void* data, size_t size);
};

} // namespace SPIFFS
} // namespace IFS
