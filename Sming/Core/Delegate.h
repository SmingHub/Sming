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
 *              Several handlers may be triggered for each event
 *  @{
 */
#pragma once

#include <user_config.h>

/** @brief  IDelegateCaller class
 *  @todo   Provide more informative brief description of IDelegateCaller
 */
template <class ReturnType, typename... ParamsList> class IDelegateCaller
{
public:
	virtual ~IDelegateCaller() = default;

	/** @brief  Invode the delegate
     *  @param  ParamList Delegate parameters
     *  @retval ReturnType Delegate return value
     */
	virtual ReturnType invoke(ParamsList...) = 0;

	/** @brief  Increase the quantity of delegate caller references by one
     */
	__forceinline void increase()
	{
		references++;
	}

	/** @brief  Decrease the quantity of delegate caller references by one
     *  @note   If no references remain the delegate caller object is deleted
     */
	__forceinline void decrease()
	{
		references--;
		if(references == 0) {
			delete this;
		}
	}

private:
	uint32_t references = 1;
};

template <class> class MethodCaller; /* undefined */

/** @brief  Delegate method caller class
*/
template <class ClassType, class ReturnType, typename... ParamsList>
class MethodCaller<ReturnType (ClassType::*)(ParamsList...)> : public IDelegateCaller<ReturnType, ParamsList...>
{
	/** @brief  Defines the return type for a delegate method
     *  @todo   Better describe delegate MethodCaller ReturnType
     */
	typedef ReturnType (ClassType::*MethodDeclaration)(ParamsList...);

public:
	/** @brief  Instantiate a delegate method caller object
     *  @param  c Pointer to the method class type
     *  @param  m Declaration of the method
     */
	MethodCaller(ClassType* c, MethodDeclaration m) : mClass(c), mMethod(m)
	{
	}

	/** @brief  Invoke the delegate method
     *  @param  args The delegate method parameters
     *  @retval ReturnType The return value from the invoked method
     */
	ReturnType invoke(ParamsList... args) override
	{
		return (mClass->*mMethod)(args...);
	}

private:
	ClassType* mClass;
	MethodDeclaration mMethod;
};

/** @brief  Delegate function caller class
*/
template <class MethodDeclaration, class ReturnType, typename... ParamsList>
class FunctionCaller : public IDelegateCaller<ReturnType, ParamsList...>
{
public:
	/** @brief  Instantiate a delegate function caller object
     *  @param  m Method declaration
     */
	FunctionCaller(MethodDeclaration m) : mMethod(m)
	{
	}

	/** @brief  Invoke the delegate function
     *  @param  args The delegate function parameters
     *  @retval ReturnType The return value from the invoked function
     */
	ReturnType invoke(ParamsList... args) override
	{
		return (mMethod)(args...);
	}

private:
	MethodDeclaration mMethod;
};

template <class> class Delegate; /* undefined */

/** @brief  Delegate class
*/
template <class ReturnType, class... ParamsList> class Delegate<ReturnType(ParamsList...)>
{
	/** @brief  Defines the return type of a delegate function declaration
     */
	typedef ReturnType (*FunctionDeclaration)(ParamsList...);

	template <typename ClassType> using MethodDeclaration = ReturnType (ClassType::*)(ParamsList...);

public:
	/** @brief  Instantiate a delegate object
    */
	__forceinline Delegate()
	{
	}

	// Class method
	/** @brief  Delegate a class method
	 *  @param m Method declaration to delegate
	 *  @param  c Pointer to the class type
	 */
	template <class ClassType> __forceinline Delegate(MethodDeclaration<ClassType> m, ClassType* c)
	{
		if(m != nullptr) {
			impl = new MethodCaller<MethodDeclaration<ClassType>>(c, m);
		}
	}

	// Function
	/** @brief  Delegate a function
	 *  @param  m Function declaration to delegate
	 */
	__forceinline Delegate(FunctionDeclaration m)
	{
		if(m != nullptr) {
			impl = new FunctionCaller<FunctionDeclaration, ReturnType, ParamsList...>(m);
		}
	}

	__forceinline ~Delegate()
	{
		if(impl != nullptr) {
			impl->decrease();
		}
	}

	/** @brief  Invoke a delegate
     *  @param  params Delegate parameters
     *  @retval ReturnType Return value from delgate
     */
	__forceinline ReturnType operator()(ParamsList... params) const
	{
		return impl->invoke(params...);
	}

	/** @brief  Move a delegate from another object
     *  @param  that Pointer to the delegate to move
     */
	__forceinline Delegate(Delegate&& that)
	{
		impl = that.impl;
		that.impl = nullptr;
	}

	/** @brief  Copy a delegate from another Delegate object
	 *  @param  that The delegate to copy
	 */
	__forceinline Delegate(const Delegate& that)
	{
		copy(that);
	}

	/** @brief  Copy a delegate from another Delegate object
     *  @param  that The delegate to copy
     *  @retval Delegate Pointer to the copied delegate
     */
	__forceinline Delegate& operator=(const Delegate& that) // copy assignment
	{
		copy(that);
		return *this;
	}

	/** @brief  Move a delegate from another Delegate object
     *  @param  that Delegate to move and assign
     *  @retval Delegate Pointer to the moved delegate
     */
	Delegate& operator=(Delegate&& that) // move assignment
	{
		if(this != &that) {
			if(impl != nullptr) {
				impl->decrease();
			}

			impl = that.impl;
			that.impl = nullptr;
		}
		return *this;
	}

	/** @brief Check for null pointer
     *  @retval bool False if null pointer
     */
	__forceinline operator bool() const
	{
		return impl != nullptr;
	}

protected:
	void copy(const Delegate& other)
	{
		if(impl != other.impl) {
			if(impl != nullptr) {
				impl->decrease();
			}
			impl = other.impl;
			if(impl != nullptr) {
				impl->increase();
			}
		}
	}

private:
	IDelegateCaller<ReturnType, ParamsList...>* impl = nullptr;
};

/** @} */
