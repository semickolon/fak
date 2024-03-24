#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define ENC_READ(n) ((!ENC_A##n << 1) | !ENC_B##n)

#ifdef SPLIT_ENABLE
#define SPLIT_KEY_COUNT_BYTES (((SPLIT_PERIPH_KEY_COUNT - 1) / 8) + 1)
#define SPLIT_ENCODER_COUNT_BYTES ((SPLIT_PERIPH_ENCODER_COUNT + 3) / 4)
#define SPLIT_MSG_REQUEST_KEYS 128
#define SPLIT_MSG_REQUEST_ENCODERS (128 + 32)
#endif

#ifdef SPLIT_SIDE_CENTRAL
#include "split_central.h"
#endif

#ifdef SPLIT_SIDE_PERIPHERAL
#include "split_peripheral.h"
#endif

#endif // __KEYBOARD_H__
