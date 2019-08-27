/*
 * Tests rational number calculations by comparing against expected results using floating point.
 */

#include "common.h"
#include <Rational.h>

class TestRational : public TestGroup
{
public:
	TestRational() : TestGroup(_F("Rational number calculations"))
	{
	}

	void execute() override
	{
		typedef uint32_t T;

		const T max = 0x10000;

		for(unsigned i = 0; i < 10000; ++i) {
			T a = os_random() % max;
			T b = os_random() % max;
			T c = os_random() % max;

			auto r = Ratio<T>(b, c);

			auto check = [&](char op, const T& res, const double& ref) {
				T rnd = (isinf(ref) || ref >= T(-1)) ? T(-1) : T(round(ref));

				if(res != rnd) {
					Serial.print(a);
					Serial.print(' ');
					Serial.print(op);
					Serial.print(" (");
					Serial.print(b);
					Serial.print(" / ");
					Serial.print(c);
					Serial.print(") = ");
					Serial.print(res);
					Serial.print(", ref = ");
					Serial.print(ref);
					Serial.print(" (");
					Serial.print(rnd);
					Serial.print(')');
					Serial.println();
				}

				TEST_ASSERT(res == rnd);
			};

			check('*', a * r, double(a) * b / c);
			check('/', a / r, double(a) * c / b);
		}
	}
};

void REGISTER_TEST(rational)
{
	registerGroup<TestRational>();
}
