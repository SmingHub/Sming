/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DirectoryStream.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 ****/

#include "DirectoryTemplate.h"
#include <DateTime.h>
#include <FlashString/Vector.hpp>

namespace IFS
{
namespace
{
#define XX(name, comment) DEFINE_FSTR_LOCAL(fieldstr_##name, #name)
DIRSTREAM_FIELD_MAP(XX)
#undef XX

#define XX(name, comment) &fieldstr_##name,
DEFINE_FSTR_VECTOR(fieldStrings, FSTR::String, DIRSTREAM_FIELD_MAP(XX))
#undef XX

} // namespace

String DirectoryTemplate::getValue(const char* name)
{
	String value = SectionTemplate::getValue(name);
	if(value) {
		return value;
	}

	int i = fieldStrings.indexOf(name);
	auto field = Field(i + 1);

	auto& d = dir();
	auto& s = d.stat();

	bool statValid = d.isValid();

	switch(field) {
	case Field::unknown:
		break;

	case Field::file_id:
		return statValid ? String(s.id) : nullptr;

	case Field::name:
		if(statValid) {
			value.setString(s.name.buffer, s.name.length);
			formatter().escape(value);
			return value;
		} else {
			return nullptr;
		}

	case Field::modified:
		return statValid ? DateTime(s.mtime).toISO8601() : nullptr;

	case Field::size:
		return statValid ? String(s.size) : nullptr;

	case Field::original_size:
		return statValid ? String(s.compression.originalSize) : nullptr;

	case Field::attr:
		return statValid ? IFS::File::getAttributeString(s.attr) : nullptr;

	case Field::attr_long:
		return statValid ? toString(s.attr) : nullptr;

	case Field::compression:
		if(!statValid) {
			return nullptr;
		} else if(!s.attr[File::Attribute::Compressed]) {
			return "";
		} else {
			return toString(s.compression.type);
		}

	case Field::access:
		return statValid ? IFS::File::getAclString(s.acl) : nullptr;

	case Field::access_long:
		return statValid ? toString(s.acl) : nullptr;

	case Field::index:
		return String(d.index());

	case Field::total_size:
		return String(d.size());

	case Field::path:
		return d.getPath();

	case Field::parent: {
		value = d.getParent();
		if(value.length() != 0) {
			formatter().escape(value);
		} else {
			value = "";
		}
		return value;
	}

	case Field::last_error:
		return d.getLastErrorString();
	}

	return nullptr;
}

} // namespace IFS
