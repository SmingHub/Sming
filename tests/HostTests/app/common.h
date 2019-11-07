/*
 * Common test code
 *
 * - A test module contains all code for testing a specific aspect of the framework
 * - If a module contains more than one file then place all files in a sub-directory
 * - Each module must have a registration function
 * -	Use the REGISTER_TEST macro to name the function
 * -	Add an entry to the `modules.h` file
 * - Each module contains one or more test groups
 * - 	Each group is a class inheriting from TestGroup
 * -	Add a call to `registerGroup<>` to the registration function for each test class
 * - Keep each group brief. Use multiple, simpler groups if necessary.
 * - The group `execute()` method is called to run the tests. On return the group is considered to
 *   have been successfully completed and destroyed.
 * - Within a group, individual tests should be marked by calling `startTest()`
 * - Call `TEST_ASSERT()` at appropriate points; passing `false` will fail the test and abort the process,
 *   displaying the source location of the failure.
 * - If a test fails then additional details may be shown before calling `TEST_ASSERT(false)`.
 * - For asynchronous testing calling `pending()` before returning. When the tests have been completed
 * 	 call `complete()`. (See `test-timers` for an example.)
 *
 * What happens:
 *
 * - The `registerGroup<>` function creates a factory function which is added to the `groupFactories` list
 * - Each group is created, executed then destroyed in turn.
 *
 * Notes
 *
 * Tests are run with DEBUG_VERBOSE_LEVEL at WARNING level, so `debug_i` statements will not normally be shown.
 * Tests can use other `debug_X` functions as required, or `Serial` print methods.
 *
 * Tests should compile and run for all architectures.
 *
 */

#pragma once

#include <Services/Profiling/MinMaxTimes.h>
#include <SmingCore.h>
using namespace Profiling;

/**
 * @brief Class to simplify generation of begin/end messages for a test group
 */
class TestGroup
{
public:
	TestGroup(const String& name) : name(name)
	{
	}

	virtual ~TestGroup()
	{
	}

	void commenceTest();

	/**
	 * @brief Implement this method to define the test
	 * @note If tests are asynchronous, call `pending()` before returning and call `complete()`
	 * when the group has completed execution (e.g. via timer callback, etc.)
	 */
	virtual void execute() = 0;

	/**
	 * @brief Note the start of a test item within a group
	 */
	void startItem(const String& tag);

	/**
	 * @brief Called when test fails to identify location
	 */
	void fail(const char* func);

	const String& getName()
	{
		return name;
	}

protected:
	void pending()
	{
		state = State::pending;
	}
	void complete();

private:
	String name;
	enum class State {
		idle,
		running,
		pending,
		failed,
	};
	State state = State::idle;
};

#define startTest(s) startItem(_F(s))

// Catch support
#define TEST_CASE(name, ...) startItem(_F(name));

/**
 * @brief Check an expression, on failure print it before assertion
 */
#define REQUIRE(expr)                                                                                                  \
	do {                                                                                                               \
		PSTR_ARRAY(tmpExprStr, #expr);                                                                                 \
		if(expr) {                                                                                                     \
			debug_i("OK: %s", tmpExprStr);                                                                             \
		} else {                                                                                                       \
			debug_e("FAIL: %s", tmpExprStr);                                                                           \
			TEST_ASSERT(false);                                                                                        \
		}                                                                                                              \
	} while(0)

/**
 * @brief Check a test result
 * @param result true if test was successful, false on failure
 * @note Failure generates an assertion so when run in the host emulator the process fails.
 */
#define TEST_ASSERT(result)                                                                                            \
	if(!(result)) {                                                                                                    \
		fail(__PRETTY_FUNCTION__);                                                                                     \
		assert(false);                                                                                                 \
	}

#define REGISTER_TEST(name) register_test_##name()

/**
 * @brief Factory function to create a TestGroup class
 */
typedef TestGroup* (*TestGroupFactory)();

/**
 * @brief List of registered class factories
 */
extern Vector<TestGroupFactory> groupFactories;

/**
 * @brief Register a factory function (a lambda) to create a given TestGroup class
 * @tparam Class to be registered
 */
template <class GroupClass> void registerGroup()
{
	groupFactories.add([]() -> TestGroup* { return new GroupClass; });
}
