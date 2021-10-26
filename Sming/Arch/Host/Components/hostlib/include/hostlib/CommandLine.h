/****
 * CommandLine.h - Provides access to command-line parameters
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

#pragma once

#include <WString.h>
#include <WVector.h>

/**
 * @defgroup hostlib Host Library
 */

/**
 * @brief Provides access to the command line
 * @ingroup hostlib
 *
 * Anything which doesn't start with ``-`` is interpreted as an application parameter.
 * For example:
 *
 * 		app param1=value
 *
 * Parameters which start with `-` are handled by the Host Emulator.
 * Anything after `--` is passed directly to the application:
 *
 * 		app -- --debug --verbose
 */
class CommandLine
{
public:
	/**
	 * @brief Manages a single parameter, may be optionally separated into name=value
	 */
	struct Parameter {
		/**
		 * @brief The text exactly as presented on the command line
		 */
		const char* text{nullptr};

		operator bool() const
		{
			return text != nullptr;
		}

		/**
		 * @brief Get parameter name, if there is one
		 */
		String getName() const;

		/**
		 * @brief Get parameter value
		 */
		String getValue() const;
	};

	/**
	 * @brief List of command-line parameters, in order
	 */
	class Parameters : public Vector<Parameter>
	{
	public:
		/**
		 * @brief Fetch parameter by name
		 * @param name Search is case-sensitive
		 */
		Parameter find(const String& name) const;

		/**
		 * @brief Fetch parameter by name
		 * @param name Search is NOT case-sensitive
		 */
		Parameter findIgnoreCase(const String& name) const;
	};

	/**
	 * @brief Fetch a reference to the list of command-line parameters
	 */
	const Parameters& getParameters()
	{
		return parameters;
	}

	/*
	 * Method called by startup code.
	 * All options have been processed so only parameters remain.
	 */
	void parse(int paramCount, char* params[]);

private:
	Parameters parameters;
};

extern CommandLine commandLine;
