#include "callbacks.h"

int testCounter;

void TestClass::callbackTest(int arg)
{
	testCounter += arg;
}

void callbackTest(int arg)
{
	testCounter += arg;
}
