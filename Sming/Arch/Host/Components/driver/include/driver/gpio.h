#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Core/Interrupts.h */

typedef enum {
	GPIO_PIN_INTR_DISABLE = 0,
	GPIO_PIN_INTR_POSEDGE = 1,
	GPIO_PIN_INTR_NEGEDGE = 2,
	GPIO_PIN_INTR_ANYEDGE = 3,
	GPIO_PIN_INTR_LOLEVEL = 4,
	GPIO_PIN_INTR_HILEVEL = 5
} GPIO_INT_TYPE;

#ifdef __cplusplus
}
#endif

