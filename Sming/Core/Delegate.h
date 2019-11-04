/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Delegate.h
 *
 ****/

/** @defgroup   delegate Delegate
 *  @brief      Delegates are event handlers
 *  @{
 */
#pragma once

#include <functional>
using namespace std::placeholders;

template <typename> class Delegate; /* undefined */

/** @brief  Delegate class
*/
template <typename ReturnType, typename... ParamTypes>
class Delegate<ReturnType(ParamTypes...)> : public std::function<ReturnType(ParamTypes...)>
{
	using StdFunc = std::function<ReturnType(ParamTypes...)>;

public:
	using StdFunc::function;

	Delegate() = default;

	/** @brief  Delegate a class method
	 *  @param m Method declaration to delegate
	 *  @param  c Pointer to the class type
	 */
	template <class ClassType>
	Delegate(ReturnType (ClassType::*m)(ParamTypes...), ClassType* c)
		: StdFunc([m, c](ParamTypes... params) -> ReturnType { return (c->*m)(params...); })
	{
	}
};

/** @} */
