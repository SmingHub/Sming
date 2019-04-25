/*
 * WiringFrameworkDependencies.h
 *
 *  Created on: 28 ���. 2015 �.
 *      Author: Anakonda
 */

#ifndef WIRING_WIRINGFRAMEWORKDEPENDENCIES_H_
#define WIRING_WIRINGFRAMEWORKDEPENDENCIES_H_

#include <user_config.h>

#include <c_types.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#define F_CPU 80000000L ////?

#include "WConstants.h"
#include "BitManipulations.h"
#include "FakePgmSpace.h"
#include "pins_arduino.h"

// std::min, etc.
#include <algorithm>

using std::isinf;
using std::isnan;
using std::max;
using std::min;


#endif /* WIRING_WIRINGFRAMEWORKDEPENDENCIES_H_ */
