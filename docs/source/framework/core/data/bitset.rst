BitSet
======

.. highlight:: c++

Introduction
------------

The C++ STL provides `std::bitset <http://www.cplusplus.com/reference/bitset/bitset/>`__
which emulates an array of ``bool`` elements.

The :cpp:class:`BitSet` class template provides similar support for sets of strongly-typed elements.
For example::

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

   static constexpr FruitBasket fixedBasket = FruitBasket(Fruit::orange) | Fruit::banana | Fruit::tomato;

A ``FruitBasket`` uses one byte of storage, with each bit representing an item of ``Fruit``.
If the basket contains a piece of fruit, the corresponding bit is set.
If it does not, the bit is clear.

.. note::

   We must add the initial ``FruitBasket(Fruit::orange)`` cast to allow the | operator to function.
   To avoid this, declare an operator like this::

      inline constexpr FruitBasket operator|(Fruit a, Fruit b)
      {
         return FruitBasket(a) | b;
      }

   From now on, we can just write::

      FruitBasket basket = Fruit::orange | Fruit::banana | Fruit::tomato;
   

Without BitSet you implement this as follows::

   using FruitBasket = uint8_t;

   static constexpr FruitBasket fixedBasket = _BV(Fruit::orange) | _BV(Fruit::banana) | _BV(Fruit::tomato);


To test whether the set contains a value you'd do this::

   if(fixedBasket & _BV(Fruit::orange)) {
      Serial.println("I have an orange");
   }

With a BitSet, you do this::

   if(fixedBasket[Fruit::orange]) {
      Serial.println("I have an orange");
   }

And you can add an element like this::

   basket[Fruit::kiwi] = true;

Bit manipulation operators are provided so you can do logical stuff like this::

   FruitBasket basket1; // Create an empty basket

   // Add a kiwi fruit
   basket1 = fixedBasket + Fruit::kiwi;

   // Create a second basket containing all fruit not in our first basket
   FruitBasket basket2 = ~basket1;

   // Remove some fruit
   basket2 -= Fruit::orange | Fruit::tomato;   

And so on.

To display the contents of a BitSet, do this::

   Serial.print(_F("My basket contains: "));
   Serial.println(basket1);

You will also need to provide an implementation of ``toString(Fruit)``
or whatever type you are using for the set elements.


API
---

.. doxygenclass:: BitSet
   :members:

