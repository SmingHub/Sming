/*
 * FlashData.cpp
 *
 *  Created on: 18 Sep 2018
 *      Author: Mike
 */

#include "FlashData.h"

#define TEXT "This is an external flash string\0two\0three\0four"

DEFINE_FSTR(externalFSTR1, TEXT)

DEFINE_PSTR(externalPSTR1, TEXT)
