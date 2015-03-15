// Just for compatibility with Arduino libraries.

#ifndef WIRING_WIRING_PRIVATE_H_
#define WIRING_WIRING_PRIVATE_H_

#include <stdio.h>
#include <stdarg.h>
#include "Arduino.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

typedef void (*voidFuncPtr)(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WIRING_WIRING_PRIVATE_H_ */
