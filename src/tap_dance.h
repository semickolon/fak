#ifndef __TAP_DANCE_H__
#define __TAP_DANCE_H__

#include "keyboard.h"

#include <stdint.h>

uint8_t is_future_type_tap_dance(uint32_t key_code);

uint8_t tap_dance_handle_event(fak_key_state_t *ks, uint8_t handle_ev, int16_t delta);

extern __code uint32_t tap_dance_bindings[];

#endif // __TAP_DANCE_H__
