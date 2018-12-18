# Embedded RingBufCPP

This is a simple ring (FIFO) buffer queuing library for embedded platforms, such as Arduino's. This library is based on a previous one I wrote, only now in C++ instead of C. It uses C++'s templating, so no more weird pointer casting, and deep copies of objects are supported. It has concurrency protection built in, so feel free to do operations
on the buffer inside of ISR's. All memory is statically allocated at compile time, so no heap memory is used. It can buffer any fixed size object (ints, floats, structs, objects, etc...).

## FAQ's
 <dl>
 <dt>I only have a C compiler for my platform</dt>
   <dd>No worries, try the <a href="https://github.com/wizard97/ArduinoRingBuffer">vanilla C version of the library</a>.</dd>
 </dl>


## Use Cases

A ring buffer is used when passing asynchronous io between two threads. In the case of the Arduino, it is very useful for buffering data in an interrupt routine that is later processed in your `void loop()`.

## Supported Platforms
The library currently supports:
- AVR
- ESP8266
- Any other platform (just implement the `RB_ATOMIC_START` and `RB_ATOMIC_END` macros)

## Install


This library is now available in the Arduino Library Manager, directly in the IDE. Go to `Sketch > Include Library > Manage Libraries` and search for `RingBufCPP`. Then `#include <RingBufCPP.h>` in your sketch.


To manually install this library, download this file as a zip, and extract the resulting folder into your Arduino Libraries folder. [Installing an Arduino Library](https://www.arduino.cc/en/Guide/Libraries).

## Examples

Look at the examples folder for several examples.

## Contributing

If you find this Arduino library helpful, click the Star button, and you will make my day.

Feel free to improve this library. Fork it, make your changes, then submit a pull request!

## API


### Constructor

```c++
RingBufCPP<typename Type, size_t MaxElements>();
```

Creates a new RingBuf object that can buffer up to MaxElements of type Type.


## Methods


### add()

```c++
bool add(Type &obj);
```

Append an element to the buffer. Return true on success, false on a full buffer.

### peek()

```c++
Type *peek(uint16_t num);
```

Peek at the num'th element in the buffer. Returns a pointer to the location of the num'th element. If num is out of bounds or the num'th element is empty, a NULL pointer is returned. Note that this gives you direct memory access to the location of the num'th element in the buffer, allowing you to directly edit elements in the buffer. Note that while all of RingBuf's public methods are atomic (including this one), directly using the pointer returned from this method is not safe. If there is a possibility an interrupt could fire and remove/modify the item pointed to by the returned pointer, disable interrupts first with `noInterrupts()`, do whatever you need to do with the pointer, then you can reenable interrupts by calling `interrupts()`.

### pull()

```c++
bool pull(Type *dest);
```

Pull the first element out of the buffer. The first element is copied into the location pointed to by dest. Returns false if the buffer is empty, otherwise returns true on success.


### numElements()
```c++
size_t numElements();
```

Returns number of elements in the buffer.

### isFull()
```c++
bool isFull();
```

Returns true if buffer is full, otherwise false.


### isEmpty()

```c++
bool isEmpty();
```

Returns true if buffer is empty, false otherwise.

## License

This library is open-source, and licensed under the [MIT license](http://opensource.org/licenses/MIT). Do whatever you like with it, but contributions are appreciated.
