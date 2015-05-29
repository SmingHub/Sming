/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_DELEGATE_H_
#define SMINGCORE_DELEGATE_H_

#include <user_config.h>

template<class ReturnType, typename... ParamsList>
class IDelegateCaller
{
public:
	virtual ~IDelegateCaller() = default;
    virtual ReturnType invoke(ParamsList...) = 0;

    __forceinline void increase()
    {
    	references++;
    }
    __forceinline void decrease()
    {
    	references--;
		if (references == 0)
			delete this;
    }
private:
    uint32_t references = 1;
};

template< class >
class MethodCaller;  /* undefined */

template< class ClassType, class ReturnType, typename... ParamsList >
class MethodCaller<ReturnType (ClassType::*)(ParamsList ...)> : public IDelegateCaller<ReturnType, ParamsList...>
{
	typedef ReturnType (ClassType::*MethodDeclaration)(ParamsList ...);
public:
    MethodCaller( ClassType* c, MethodDeclaration m ) : mClass( c ), mMethod( m ) {}
    ReturnType invoke(ParamsList... args)
    {
    	return (mClass->*mMethod)( args... );
    }

private:
    ClassType *mClass;
    MethodDeclaration mMethod;
};

template< class MethodDeclaration, class ReturnType, typename... ParamsList >
class FunctionCaller : public IDelegateCaller<ReturnType, ParamsList...>
{
public:
    FunctionCaller( MethodDeclaration m ) : mMethod( m ) {}
    ReturnType invoke(ParamsList... args)
    {
    	return (mMethod)( args... );
    }

private:
    MethodDeclaration mMethod;
};

template <class>
class Delegate; /* undefined */

template<class ReturnType, class ... ParamsList>
class Delegate <ReturnType (ParamsList ...)>
{
	typedef ReturnType (*FunctionDeclaration)(ParamsList...);
	template<typename ClassType> using MethodDeclaration = ReturnType (ClassType::*)(ParamsList ...);

public:
	__forceinline Delegate()
	{
		impl = nullptr;
	}

	// Class method
	template <class ClassType>
	__forceinline Delegate(MethodDeclaration<ClassType> m, ClassType* c)
	{
		if (m != NULL)
			impl = new MethodCaller< MethodDeclaration<ClassType> >(c, m);
		else
			impl = nullptr;
	}

	// Function
	__forceinline Delegate(FunctionDeclaration m)
	{
		if (m != NULL)
			impl = new FunctionCaller< FunctionDeclaration, ReturnType, ParamsList... >(m);
		else
			impl = nullptr;
	}

	__forceinline ~Delegate()
    {
    	if (impl != nullptr)
    		impl->decrease();
    }

    __forceinline ReturnType operator()(ParamsList... params) const
    {
        return impl->invoke(params...);
    }

    __forceinline Delegate(Delegate&& that)
    {
    	impl = that.impl;
		that.impl = nullptr;
	}
    __forceinline Delegate(const Delegate& that)
    {
    	copy(that);
    }
    __forceinline Delegate& operator=(const Delegate& that) // copy assignment
    {
		copy(that);
        return *this;
    }
    Delegate& operator=(Delegate&& that) // move assignment
	{
    	if (this != &that)
		{
			if (impl)
				impl->decrease();

			impl = that.impl;
			that.impl = nullptr;
		}
		return *this;
	}

    // Check for nullptr
    __forceinline operator bool() const
    {
        return impl != nullptr;
    }

protected:
    void copy(const Delegate& other)
	{
		if (impl != other.impl)
		{
			if (impl)
				impl->decrease();
			impl = other.impl;
			if (impl)
				impl->increase();
		}
	}

private:
    IDelegateCaller<ReturnType, ParamsList...>* impl = nullptr;
};


#endif /* SMINGCORE_DELEGATE_H_ */
