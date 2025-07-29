/*
 * Tests rational number calculations by comparing against expected results using floating point.
 */

#include <HostTests.h>
#include <Rational.h>

class TestRational : public TestGroup
{
public:
	TestRational() : TestGroup(_F("Rational number calculations"))
	{
	}

	void execute() override
	{
		using T = uint32_t;

		const T max = 0x10000;

		for(unsigned i = 0; i < 10000; ++i) {
			T a = os_random() % max;
			T b = 1 + os_random() % max;
			T c = 1 + os_random() % max;

			auto r = Ratio<T>(b, c);

			auto check = [&](char op, const T& res, const double& ref) {
				T rnd = (isinf(ref) || ref >= T(-1)) ? T(-1) : T(round(ref));

				if(res != rnd) {
					Serial << a << ' ' << op << " (" << b << " / " << c << ") = " << res << ", ref = " << ref << " ("
						   << rnd << ')' << endl;
				}

				TEST_ASSERT(res == rnd);
			};

			check('*', a * r, double(a) * b / c);
			check('/', a / r, double(a) * c / b);
		}
	}
};

void REGISTER_TEST(Rational)
{
	registerGroup<TestRational>();
}
