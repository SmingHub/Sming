/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SectionTemplate.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#include "SectionTemplate.h"
#include "TemplateStream.h"
#include <FlashString/Vector.hpp>
#include <Network/WebConstants.h>

namespace
{
#define XX(name, comment) DEFINE_FSTR_LOCAL(cmdstr_##name, #name)
SECTION_TEMPLATE_COMMAND_MAP(XX)
#undef XX

#define XX(name, comment) &cmdstr_##name,
DEFINE_FSTR_VECTOR(commandStrings, FSTR::String, SECTION_TEMPLATE_COMMAND_MAP(XX))
#undef XX

#define XX(name, comment) DEFINE_FSTR_LOCAL(fldstr_##name, #name)
SECTION_TEMPLATE_FIELD_MAP(XX)
#undef XX

#define XX(name, comment) &fldstr_##name,
DEFINE_FSTR_VECTOR(fieldStrings, FSTR::String, SECTION_TEMPLATE_FIELD_MAP(XX))
#undef XX

char* findSep(char* ptr)
{
	while(*ptr != '\0' && *ptr != ':' && *ptr != '}') {
		++ptr;
	}
	return ptr;
}

bool isNumber(const char* s)
{
	return (s != nullptr) && ((*s == '-') || (*s == '+') || isdigit(*s));
}

bool isNumber(const String& s)
{
	return isNumber(s.c_str());
}

bool isFloat(const char* s)
{
	return isNumber(s) && strchr(s, '.') != nullptr;
}

bool isFloat(const String& s)
{
	return isFloat(s.c_str());
}

static constexpr uint8_t maxArgs{8};
struct Arg {
	enum class Type {
		number,
		string,
		variable,
		expression,
	};
	void* ptr{};
	Type type{};

	~Arg()
	{
		if(type == Type::expression) {
			delete static_cast<String*>(ptr);
		}
	}
};

class ArgList
{
public:
	class ArgRef
	{
	public:
		ArgRef(SectionTemplate& tmpl, Arg* arg) : tmpl(tmpl), arg(arg)
		{
		}

		operator String() const
		{
			return toString();
		}

		String toString() const
		{
			if(arg == nullptr) {
				return nullptr;
			}

			switch(arg->type) {
			case Arg::Type::number:
			case Arg::Type::string:
				return static_cast<char*>(arg->ptr);
			case Arg::Type::variable:
				return tmpl.getValue(static_cast<const char*>(arg->ptr));
			case Arg::Type::expression:
				return *static_cast<String*>(arg->ptr);
			default:
				return nullptr;
			}
		}

		char asChar() const
		{
			return (arg == nullptr) ? '\0' : *static_cast<char*>(arg->ptr);
		}

		int toInt() const
		{
			return toString().toInt();
		}

		float toFloat() const
		{
			return toString().toFloat();
		}

		int compare(const ArgRef& other) const
		{
			String s1 = toString();
			String s2 = other.toString();
			if(isNumber(s1.c_str())) {
				return s1.toInt() - s2.toInt();
			} else {
				return s1.compareTo(s2);
			}
		}

		String add(const ArgRef& other) const
		{
			String s = toString();
			if(isFloat(s)) {
				return String(s.toFloat() + String(other).toFloat());
			}
			if(isNumber(s)) {
				return String(s.toInt() + String(other).toInt());
			}
			return s + String(other);
		}

		String sub(const ArgRef& other) const
		{
			String s = toString();
			if(isFloat(s)) {
				return String(s.toFloat() - other.toFloat());
			}
			if(isNumber(s)) {
				return String(s.toInt() - other.toInt());
			}
			return s + String(other);
		}

	private:
		SectionTemplate& tmpl;
		Arg* arg;
	};

	ArgList(SectionTemplate& tmpl) : tmpl(tmpl)
	{
	}

	ArgRef operator[](uint8_t i)
	{
		return (i >= argCount) ? ArgRef(tmpl, nullptr) : ArgRef(tmpl, &args[i]);
	}

	char* parse(char* ptr)
	{
		while(argCount < maxArgs) {
			/*
			 * Parse either:
			 * 	varname			name:12...		->		valueof(name)
			 * 	raw value		15:18...		->		15
			 * 	quoted value	"text":arg...	->		text
			 *
			 * 	ptr is left at separator
			 */
			if(*ptr == '\0' || *ptr == '}') {
				break;
			}

			auto& arg = args[argCount++];
			if(*ptr == '"') {
				++ptr;
				arg.type = Arg::Type::string;
				arg.ptr = ptr;
				auto endQuote = strchr(ptr, '"');
				if(endQuote == nullptr) {
					// Malformed {cmd:"value}
					ptr += strlen(ptr);
				} else {
					*endQuote = '\0';
					ptr = endQuote + 1;
				}
			} else {
				if(*ptr == '{') {
					++ptr;
					arg.type = Arg::Type::expression;
					arg.ptr = new String(tmpl.evaluate(ptr));
				} else if(isNumber(ptr)) {
					arg.type = Arg::Type::number;
					arg.ptr = ptr;
				} else {
					arg.type = Arg::Type::variable;
					arg.ptr = ptr;
				}
				ptr = findSep(ptr);
			}

			char c = *ptr;
			*ptr++ = '\0';
			if(c != ':') {
				break;
			}
		}

		return ptr;
	}

	uint8_t count() const
	{
		return argCount;
	}

private:
	SectionTemplate& tmpl;
	Arg args[maxArgs];
	uint8_t argCount{0};
};

} // namespace

SectionTemplate::SectionTemplate(IDataSourceStream* source)
	: TemplateStream(&sectionStream, false), sectionStream(source)
{
	setFormatter(Format::standard);
	sectionStream.onNextSection([this]() { seekFrom(0, SeekOrigin::Start); });
	sectionStream.onNextRecord(SectionStream::NextRecord(&SectionTemplate::nextRecord, this));
	// Level 0 is always enabled
	conditionalFlags[0] = true;
}

String SectionTemplate::openTag(bool enable)
{
	++conditionalLevel;
	conditionalFlags.set(conditionalLevel, enable);
	enableOutput(enable);
	return "";
}

String SectionTemplate::closeTag()
{
	if(conditionalLevel == 0) {
		return nullptr;
	}
	--conditionalLevel;
	auto enable = conditionalFlags[conditionalLevel];
	enableOutput(enable);

	if(conditionalLevel == 0 && newSection >= 0) {
		debug_e("NewSection: %d", newSection);
		sectionStream.setNewSection(newSection);
		newSection = -1;
	}

	return "";
}

String SectionTemplate::elseTag()
{
	if(newSection >= 0) {
		debug_e("NewSection: %d", newSection);
		sectionStream.setNewSection(newSection);
		newSection = -1;
	}

	if(conditionalLevel == 0) {
		return nullptr;
	}
	conditionalFlags.flip(conditionalLevel);
	auto enable = conditionalFlags[conditionalLevel];
	enableOutput(enable);
	return "";
}

String SectionTemplate::evaluate(char*& expr)
{
	if(*expr != '!') {
		auto sep = findSep(expr);
		if(sep == nullptr) {
			sep = expr + strlen(expr);
		} else {
			*sep = '\0';
		}
		String s = getValue(expr);
		*sep = '_';
		expr = sep + 1;
		return s;
	}

	auto exprStart = expr;
	*expr = 'Q';

	// Process conditional expressions such as {length:varname}, {width:10:text to pad}
	ArgList args(*this);

	auto ptr = findSep(expr + 1);
	if(*ptr == '}') {
		*ptr++ = '\0';
	} else if(*ptr != '\0') {
		// {cmd} or {cmd:value...}
		*ptr++ = '\0'; // Terminate cmd
		ptr = args.parse(ptr);
	}

	int i = commandStrings.indexOf(expr);
	auto field = Command(i + 1);

	expr = ptr;

	switch(field) {
	case Command::Qunknown:
		break;

	case Command::Qas_int:
		return String(args[0].toInt());

	case Command::Qas_float:
		return String(args[0].toFloat());

	case Command::Qas_string: {
		/*
		 * TODO: Find/write strQuote() helper function to ensure any containing quotes are escaped.
		 * This will be format-specific so perhaps another callback or look at putting Stream/Format
		 * functions into classes.
		 */
		String s;
		s += '"';
		s += args[0];
		s += '"';
		return s;
	}

	case Command::Qkb:
		return String(args[0].toFloat() / 1024);

	case Command::Qreplace: {
		String value = args[0];
		value.replace(args[1], args[2]);
		return value;
	}

	case Command::Qlength:
		return String(args[0].toString().length());

	case Command::Qmime_type: {
		String s = ContentType::fromFullFileName(args[0]);
		return s ?: "";
	}

	case Command::Qpad: {
		String value = args[0];
		int strlen = value.length();
		int padlen = args[1].toInt();
		char c = (args.count() >= 3) ? args[2].asChar() : ' ';
		if(padlen < 0) {
			padlen = -padlen;
			int diff = padlen - strlen;
			if(diff > 0) {
				value.setLength(padlen);
				memmove(&value[diff], &value[0], strlen);
				memset(&value[0], c, diff);
			}
		} else if(padlen > strlen) {
			value.setLength(padlen);
			memset(&value[strlen], c, padlen - strlen);
		}
		return value;
	}

	case Command::Qrepeat: {
		int count = args[1].toInt();
		if(count <= 0) {
			return "";
		}
		String value = args[0];
		auto len = value.length();
		value.reserve(len * count);
		while(count-- > 1) {
			value.concat(value.c_str(), len);
		}
		return value;
	}

	case Command::Qifdef:
		return openTag(args[0].toString().length() != 0);

	case Command::Qifndef:
		return openTag(args[0].toString().length() == 0);

	case Command::Qifeq:
		return openTag(args[0].compare(args[1]) == 0);

	case Command::Qifneq:
		return openTag(args[0].compare(args[1]) != 0);

	case Command::Qifgt:
		return openTag(args[0].compare(args[1]) > 0);

	case Command::Qiflt:
		return openTag(args[0].compare(args[1]) < 0);

	case Command::Qifge:
		return openTag(args[0].compare(args[1]) >= 0);

	case Command::Qifle:
		return openTag(args[0].compare(args[1]) <= 0);

	case Command::Qifbtw:
		return openTag(args[0].compare(args[1]) >= 0 && args[0].compare(args[2]) <= 0);

	case Command::Qifin:
		return openTag(String(args[0]).indexOf(args[1]) >= 0);

	case Command::Qifnin:
		return openTag(String(args[0]).indexOf(args[1]) < 0);

	case Command::Qadd:
		return args[0].add(args[1]);

	case Command::Qsub:
		return args[0].sub(args[1]);

	case Command::Qelse:
		return elseTag();

	case Command::Qendif:
		return closeTag();

	case Command::Qgoto:
		if(isOutputEnabled()) {
			int n = args[0].toInt();
			if(unsigned(n) >= sectionStream.count()) {
				return nullptr;
			}
			debug_e("goto: %d, recordIndex = %d", n, recordIndex());
			newSection = n;
		}
		return "";

	case Command::Qcount: {
		auto section = sectionStream.getSection(args[0].toInt());
		if(section == nullptr) {
			return nullptr;
		}
		return String(section->recordCount);
	}

	case Command::Qindex: {
		auto section = sectionStream.getSection(args[0].toInt());
		if(section == nullptr) {
			return nullptr;
		}
		return String(section->recordIndex);
	}
	}

	// Fix any nulls before returning
	for(; exprStart != expr; ++exprStart) {
		if(*exprStart == '\0') {
			*exprStart = '!';
		}
	}

	return nullptr;
}

String SectionTemplate::getValue(const char* name)
{
	if(*name == '$') {
		++name;
		int i = fieldStrings.indexOf(name);
		auto field = Field(i + 1);

		switch(field) {
		case Field::section:
			return String(sectionIndex());

		case Field::record:
			return String(recordIndex());

		case Field::unknown:
			break;
		}
	}

	String s;
	if(getValueCallback) {
		s = getValueCallback(name);
		if(s) {
			return s;
		}
	}

	return TemplateStream::getValue(name);
}

bool SectionTemplate::gotoSection(uint8_t index)
{
	if(!sectionStream.gotoSection(index)) {
		return false;
	}
	if(seekFrom(0, SeekOrigin::Start) != 0) {
		return false;
	}
	conditionalLevel = 0;
	return true;
}
