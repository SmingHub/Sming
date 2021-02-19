#include <HostTests.h>
#include <Data/BitSet.h>

#define FRUIT_ELEMENT_MAP(XX)                                                                                          \
	XX(apple)                                                                                                          \
	XX(banana)                                                                                                         \
	XX(kiwi)                                                                                                           \
	XX(orange)                                                                                                         \
	XX(passion)                                                                                                        \
	XX(pear)                                                                                                           \
	XX(tomato)

enum class Fruit {
#define XX(n) n,
	FRUIT_ELEMENT_MAP(XX)
#undef XX
		MAX
};

#define XX(n) #n "\0"
DEFINE_FSTR_LOCAL(fruitStrings, FRUIT_ELEMENT_MAP(XX))
#undef XX

String toString(Fruit f)
{
	return CStringArray(fruitStrings)[unsigned(f)];
}

using FruitBasket = BitSet<uint8_t, Fruit, size_t(Fruit::MAX)>;

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
			Serial.print(_F("fixedBasket contains: "));
			Serial.println(toString(fixedBasket));

			FruitBasket basket;
			REQUIRE(basket.value() == 0);
			REQUIRE(!basket);

			basket += Fruit::pear | Fruit::tomato;
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

			basket |= Fruit::kiwi;
			REQUIRE(basket.value() == (_BV(Fruit::kiwi) | _BV(Fruit::banana) | _BV(Fruit::tomato)));

			basket &= fixedBasket;
			REQUIRE(basket.value() == (_BV(Fruit::banana) | _BV(Fruit::tomato)));

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
			using NumberSet = BitSet<uint32_t, uint8_t>;
			NumberSet numbers = 0x12345678U;
			Serial.print(_F("numbers = "));
			Serial.println(toString(numbers));
			REQUIRE(numbers.value() == 0x12345678U);

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

void REGISTER_TEST(BitSet)
{
	registerGroup<BitSetTest>();
}
