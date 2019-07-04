#include <SmingCore.h>

void evaluateSpeed();

void plainOldOrdinaryFunction()
{
	debugf("plainOldOrdinaryFunction");
}

void functionWithMoreComplicatedSignature(int a, String b)
{
	debugf("functionWithMoreComlicatedSignature %d %s", a, b.c_str());
}

class Task
{
public:
	Task(){};
	bool setTimer(int reqInterval)
	{
		if(reqInterval <= 0) {
			return false;
		}
		taskInterval = reqInterval;
		return true;
	}

	// This example shows how to use a plain old ordinary function as a callback
	void callPlainOldOrdinaryFunction()
	{
		taskTimer.initializeMs(taskInterval, plainOldOrdinaryFunction).start();
		// or just
		// taskTimer.initializeMs(taskInterval, plainOldOrdinaryFunction).start();
	}

	// This example shows how to use std::bind to make us of a function that has more parameters than our signature has
	void showHowToUseBind()
	{
		auto b = std::bind(functionWithMoreComplicatedSignature, 2, "parameters");
		taskTimer.initializeMs(taskInterval, b).start();
	}

	// This example shows how to use a lamda expression as a callback
	void callLamda()
	{
		int foo = 123;
		taskTimer
			.initializeMs(
				taskInterval,
				[foo] // capture just foo by value (Note it would be bad to pass by reference as foo would be out of scope when the lamda function runs later)
				()	// No parameters to the callback
				-> void // Returns nothing
				{
					if(foo == 123) {
						debugf("lamda Callback foo is 123");
					} else {
						debugf("lamda Callback foo is not 123, crikey!");
					}
				})
			.start();
	}

	// This example shows how to use a member function as a callback
	void callMemberFunction()
	{
		// A non-static member function must be called with an object.
		// That is, it always implicitly passes "this" pointer as its argument.
		auto b = TimerDelegate(&Task::callbackMemberFunction, this);
		taskTimer.initializeMs(taskInterval, b).start();
	}

	void callbackMemberFunction()
	{
		debugf("callMemberFunction");
	}

private:
	Timer taskTimer;
	int taskInterval = 1000;
};

Task task1;
Task task2;
Task task3;
Task task4;
Task task5;

void init()
{
	Serial.begin(COM_SPEED_SERIAL);

	evaluateSpeed();

	task2.setTimer(1600);
	task2.callPlainOldOrdinaryFunction();

	task3.setTimer(1900);
	task3.showHowToUseBind();

	task4.setTimer(1700);
	task4.callMemberFunction();

	task5.setTimer(1800);
	task5.callLamda();
}
