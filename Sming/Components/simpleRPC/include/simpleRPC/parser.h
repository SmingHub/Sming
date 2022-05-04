/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * parser.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#include <Delegate.h>

namespace simpleRPC
{
enum class ParserResult { finished = 0, more, error };

enum class ParserState {
	ready = 0,
	header_s,
	header_si,
	header_sim,
	header_simp,
	header_simpl,
	header_simple,
	header_simpleR,
	header_simpleRP,
	header_simpleRPC,
	header_end,
	version_major,
	version_minor,
	version_patch,
	ness,
	type,
	type_end,
	start_methods,
	extract_method_start,
	extract_method_signature,
	extract_method_name,
	extract_method,
	extract_method_end,
	end_methods,
	finished
};

struct ParserSettings {
	using SimpleMethod = Delegate<void(void)>;
	using CharMethod = Delegate<void(char)>;

	SimpleMethod startMethods;
	SimpleMethod startMethod;
	CharMethod methodSignature;
	CharMethod methodName;
	SimpleMethod endMethod;
	SimpleMethod endMethods;
	ParserState state = ParserState::ready;
};

ParserResult parse(ParserSettings& settings, const char* buffer, size_t length, char nameEndsWith = ':');

} // namespace simpleRPC
