/**
 * CommandLine.cpp
 *
 * Copyright 2020 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/hostlib/CommandLine.h"

CommandLine commandLine;

String CommandLine::Parameter::getName() const
{
	if(text == nullptr) {
		return nullptr;
	}

	auto sep = strchr(text, '=');
	if(sep == nullptr) {
		return nullptr;
	} else {
		return String(text, sep - text);
	}
}

String CommandLine::Parameter::getValue() const
{
	if(text == nullptr) {
		return nullptr;
	}

	auto sep = strchr(text, '=');
	if(sep == nullptr) {
		return text;
	} else {
		return String(sep + 1);
	}
}

/**
 * @brief Fetch parameter by name
 * @param name Search is case-sensitive
 */
CommandLine::Parameter CommandLine::Parameters::find(const String& name) const
{
	for(unsigned i = 0; i < count(); ++i) {
		auto& param = elementAt(i);
		if(name.equals(param.getName())) {
			return param;
		}
	}

	return Parameter{};
}

/**
 * @brief Fetch parameter by name
 * @param name Search is NOT case-sensitive
 */
CommandLine::Parameter CommandLine::Parameters::findIgnoreCase(const String& name) const
{
	for(unsigned i = 0; i < count(); ++i) {
		auto& param = elementAt(i);
		if(name.equalsIgnoreCase(param.getName())) {
			return param;
		}
	}

	return Parameter{};
}

void CommandLine::parse(int paramCount, char* params[])
{
	parameters.clear();
	for(int i = 0; i < paramCount; ++i) {
		parameters.addElement(new Parameter{params[i]});
	}
}
