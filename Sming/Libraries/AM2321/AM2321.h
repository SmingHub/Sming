// 
// AM2321 Temperature & Humidity Sensor library for Arduino
// VERSION: 0.1.0
//
// The MIT License (MIT)
//
// Copyright (c) 2013 Wang Dong
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef __ARDUINO_AM2321_H__
#define __ARDUINO_AM2321_H__

#include <Arduino.h>

#define LIBAM2321_VERSION "0.1.0"


class AM2321
{
public:
    int temperature;
    unsigned int humidity;

public:
    void begin();
    bool read();
    bool available();

public:
    uint32_t uid();

public:
    AM2321();
};


#endif
//
// END OF FILE
//
