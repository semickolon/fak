#ifndef __HOLD_TAP_H__
#define __HOLD_TAP_H__

#include "keyboard.h"
#include "keymap.h"

#include <stdint.h>

#define HOLD_TAP_KEY_INTERRUPT_ENABLE 0x01
#define HOLD_TAP_KEY_INTERRUPT_DECIDE_HOLD 0x02
#define HOLD_TAP_KEY_INTERRUPT_DECIDE_TAP 0x00
#define HOLD_TAP_KEY_INTERRUPT_ON_PRESS 0x04
#define HOLD_TAP_KEY_INTERRUPT_ON_RELEASE 0x00

#define HOLD_TAP_FLAGS_TIMEOUT_DECISION_MASK 0x01
#define HOLD_TAP_FLAGS_TIMEOUT_DECISION_TAP 0x00
#define HOLD_TAP_FLAGS_TIMEOUT_DECISION_HOLD 0x01

#ifdef HOLD_TAP_EAGER_ENABLE
#define HOLD_TAP_FLAGS_EAGER_MASK 0x06
#define HOLD_TAP_FLAGS_EAGER_NONE 0x00
#define HOLD_TAP_FLAGS_EAGER_HOLD 0x02
#define HOLD_TAP_FLAGS_EAGER_TAP 0x04
#endif

#ifdef HOLD_TAP_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE_ENABLE
#define HOLD_TAP_FLAGS_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE 0x08
#endif

uint8_t is_future_type_hold_tap(uint32_t key_code);

uint8_t hold_tap_handle_event(fak_key_state_t *ks, uint8_t handle_ev, int16_t delta);

typedef struct {
    uint8_t flags;
    uint16_t timeout_ms;
    uint8_t key_interrupts[(KEY_COUNT + 1) / 2];
#ifdef HOLD_TAP_QUICK_TAP_ENABLE
    uint8_t quick_tap_ms;
#ifdef HOLD_TAP_QUICK_TAP_INTERRUPT_ENABLE
    uint16_t quick_tap_interrupt_ms;
#endif
#endif
#ifdef HOLD_TAP_GLOBAL_QUICK_TAP_ENABLE
    uint16_t global_quick_tap_ms;
#endif
} fak_hold_tap_behavior_t;

extern __code fak_hold_tap_behavior_t hold_tap_behaviors[];

#endif // __HOLD_TAP_H__
