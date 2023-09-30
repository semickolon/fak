#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#ifdef SPLIT_ENABLE
#define SPLIT_MSG_REQUEST_KEYS 69
#define SPLIT_KEY_COUNT_BYTES (((SPLIT_PERIPH_KEY_COUNT - 1) / 8) + 1)
#endif

#ifdef SPLIT_SIDE_CENTRAL
#include "split_central.h"
#endif

#ifdef SPLIT_SIDE_PERIPHERAL
#include "split_peripheral.h"
#endif

#endif // __KEYBOARD_H__
