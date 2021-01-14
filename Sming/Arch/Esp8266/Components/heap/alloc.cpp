/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * alloc.cpp - C++ heap allocators
 *
 ****/

#include <cstdlib>
#include <new>

void* operator new(size_t size)
{
	return malloc(size);
}

void* operator new(size_t size, const std::nothrow_t&)
{
	return malloc(size);
}

void* operator new[](size_t size)
{
	return malloc(size);
}

void* operator new[](size_t size, const std::nothrow_t&)
{
	return malloc(size);
}

void operator delete(void* ptr)
{
	free(ptr);
}

void operator delete[](void* ptr)
{
	free(ptr);
}

void operator delete(void* ptr, size_t sz)
{
	free(ptr);
}

void operator delete[](void* ptr, size_t sz)
{
	free(ptr);
}
