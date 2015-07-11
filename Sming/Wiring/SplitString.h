/* $Id: WMemory.cpp 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | Implementation of c++ new/delete operators.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef _SPLIT_STRING_H_
#define _SPLIT_STRING_H_

#include "WVector.h"
#include "WString.h"
#include "WiringFrameworkDependencies.h"

int splitString(String &what, int delim,  Vector<long> &splits);
int splitString(String &what, int delim,  Vector<int> &splits);
int splitString(String &what, int delim,  Vector<String> &splits);

#endif
