#ifndef __SPLIT_PERIPHERAL_H__
#define __SPLIT_PERIPHERAL_H__

#include <stdint.h>

void key_state_inform(uint8_t key_idx, uint8_t down);
#if SPLIT_ENCODER_COUNT_BYTES > 0
void encoder_scan(uint8_t i, uint8_t reading);
#endif

void keyboard_init();
void keyboard_scan();

#endif // __SPLIT_PERIPHERAL_H__
