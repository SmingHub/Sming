#include <SmingTest.h>
#include <Data/BitSet.h>

namespace
{
enum class Fruit {
	apple,
	banana,
	kiwi,
	orange,
	passion,
	pear,
	tomato,
};

using FruitBasket = BitSet<uint8_t, Fruit, unsigned(Fruit::tomato) + 1>;
using NumberSet = BitSet<uint32_t, uint8_t>;

static constexpr FruitBasket fixedBasket = Fruit::orange | Fruit::banana | Fruit::tomato;

/*
 * BitSet is safe to use embedded in structures.
 * It can therefore be used in external interface definitions, or in data structures
 * which are sent 'over the wire'.
 */
struct Struct {
	FruitBasket basket1{fixedBasket};
	uint8_t value;
	FruitBasket basket2;
};

static_assert(sizeof(fixedBasket) == 1, "fixedBasket size wrong");
static_assert(sizeof(Struct) == 3, "Struct size wrong");

/*
// Examples of declarations which will fail compilation

BitSet<uint8_t, uint8_t, 9> badBitSet1;
BitSet<uint32_t, uint8_t, 0> badBitSet2;
BitSet<uint32_t, int, 5> badBitSet3;
BitSet<uint32_t, float, 5> badBitSet4;
BitSet<float, uint8_t, 9> badBitSet5;

*/

}; // namespace

class BitSetTest : public TestGroup
{
public:
	BitSetTest() : TestGroup(_F("BitSet"))
	{
	}

	void execute() override
	{
		TEST_CASE("constexpr")
		{
			REQUIRE(fixedBasket.value() == (_BV(Fruit::orange) | _BV(Fruit::banana) | _BV(Fruit::tomato)));
		}

		TEST_CASE("Operations")
		{
			FruitBasket basket;
			REQUIRE(basket.value() == 0);
			REQUIRE(!basket);

			basket += Fruit::pear + Fruit::tomato;
			REQUIRE(basket);
			REQUIRE(basket.value() == (_BV(Fruit::pear) | _BV(Fruit::tomato)));

			basket += fixedBasket;
			REQUIRE(basket.value() == (fixedBasket.value() | _BV(Fruit::pear)));

			FruitBasket basket2 = Fruit::pear;
			debug_i("basket = 0x%08x", basket.value());
			debug_i("fixedBasket = 0x%08x", fixedBasket.value());
			REQUIRE(basket != fixedBasket);
			REQUIRE(!(basket == fixedBasket));
			basket -= basket2;
			REQUIRE(basket == fixedBasket);

			FruitBasket empty;
			REQUIRE(!(basket == empty));
			REQUIRE(basket != empty);

			basket -= Fruit::orange;
			REQUIRE(basket.value() == (_BV(Fruit::banana) | _BV(Fruit::tomato)));

			basket &= fixedBasket;

			basket = ~fixedBasket;
			debug_e("basket.value = 0x%08x", basket.value());
			REQUIRE(basket.value() == (_BV(Fruit::apple) | _BV(Fruit::kiwi) | _BV(Fruit::passion) | _BV(Fruit::pear)));

			REQUIRE(basket[Fruit::apple]);
			REQUIRE(!basket[Fruit::tomato]);

			REQUIRE(basket.domain().value() ==
					(_BV(Fruit::apple) | _BV(Fruit::banana) | _BV(Fruit::kiwi) | _BV(Fruit::orange) |
					 _BV(Fruit::passion) | _BV(Fruit::pear) | _BV(Fruit::tomato)));
		}

		TEST_CASE("Number set")
		{
			NumberSet numbers = 12U;
			REQUIRE(numbers.value() == 12);

			numbers = NumberSet{};
			REQUIRE(numbers.value() == 0);

			numbers |= uint8_t(1);
			REQUIRE(numbers.value() == _BV(1));

			numbers |= uint8_t(5);
			numbers |= uint8_t(10);
			REQUIRE(numbers.value() == (_BV(1) | _BV(5) | _BV(10)));
		}

		TEST_CASE("64-bit set")
		{
			BitSet<uint64_t, uint8_t, 35> large;

			REQUIRE(sizeof(large) == 8);
			REQUIRE(large.domain().value() == 0x7FFFFFFFFULL);
		}
	}
};

void REGISTER_TEST(bitset)
{
	registerGroup<BitSetTest>();
}
