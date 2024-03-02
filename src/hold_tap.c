#include "hold_tap.h"
#include "time.h"
#include "usb.h"
#include "key_event_queue.h"

#define STATE_DEFAULT 0
#define STATE_PRE_QUICK_TAP 1
#define STATE_POST_QUICK_TAP 2

#if defined(HOLD_TAP_GLOBAL_QUICK_TAP_ENABLE) && defined(HOLD_TAP_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE_ENABLE)
#define STATE_IMMEDIATE_TAP 3
#define STATE_POST_IMMEDIATE_TAP 4
#endif

#define EAGER_START 1
#define EAGER_DECIDE_TAP 2
#define EAGER_DECIDE_HOLD 3

#define TEMP_TAP(ks, down) handle_non_future(ks->key_code & KEY_CODE_TAP_MASK, down);
#define TEMP_HOLD(ks, down) handle_non_future(ks->key_code & KEY_CODE_HOLD_LAYER_IDX_MODS_MASK, down);

uint8_t is_future_type_hold_tap(uint32_t key_code) {
    return (key_code & KEY_CODE_HOLD_MASK) && (key_code & KEY_CODE_TAP_MASK);
}

static uint8_t get_key_interrupt(fak_hold_tap_behavior_t *behavior, uint8_t key_idx) {
    uint8_t interrupt = behavior->key_interrupts[key_idx / 2];
    if (key_idx % 2) {
        return (interrupt >> 4);
    } else {
        return (interrupt & 0x0F);
    }
}

// Returns true if the decision matches the eager action
static uint8_t process_eager(
    fak_key_state_t *ks,
    uint8_t behavior_flags,
    uint8_t event
) {
#ifdef HOLD_TAP_EAGER_ENABLE
    switch (behavior_flags & HOLD_TAP_FLAGS_EAGER_MASK) {
    case HOLD_TAP_FLAGS_EAGER_HOLD:
        switch (event) {
        case EAGER_START: TEMP_HOLD(ks, 1); break;
        case EAGER_DECIDE_TAP: TEMP_HOLD(ks, 0); break;
        case EAGER_DECIDE_HOLD: return 1;
        }
        break;
    case HOLD_TAP_FLAGS_EAGER_TAP:
        switch (event) {
        case EAGER_START: TEMP_TAP(ks, 1); break;
        case EAGER_DECIDE_HOLD: TEMP_TAP(ks, 0); break;
        case EAGER_DECIDE_TAP: return 1;
        }
        break;
    }
#endif
    return 0;
}

static uint8_t resolve_eager(uint8_t decide_hold, fak_key_state_t *ks, uint8_t behavior_flags) {
    uint8_t eager_correct;

    if (decide_hold) {
        eager_correct = process_eager(ks, behavior_flags, EAGER_DECIDE_HOLD);
        ks->key_code &= KEY_CODE_HOLD_LAYER_IDX_MODS_MASK;
    } else {
        eager_correct = process_eager(ks, behavior_flags, EAGER_DECIDE_TAP);
        ks->key_code &= KEY_CODE_TAP_MASK;
    }

    if (eager_correct) {
        ks->status |= KEY_STATUS_RESOLVED;
        return HANDLE_RESULT_COMPLETED;
    }

    return HANDLE_RESULT_MAPPED;
}

uint8_t hold_tap_handle_event(fak_key_state_t *ks, uint8_t handle_event, int16_t delta) {
    fak_key_event_t *ev_front = key_event_queue_front();
    uint8_t behavior_idx = (ks->key_code & KEY_CODE_HOLD_BEHAVIOR_MASK) >> 29;
    __code fak_hold_tap_behavior_t *behavior = &hold_tap_behaviors[behavior_idx];
    uint8_t *state = key_event_queue_state();

    if (delta) {
        if (
            *state == STATE_DEFAULT &&
            behavior->timeout_ms && delta >= behavior->timeout_ms
        ) {
            uint8_t decision = behavior->flags & HOLD_TAP_FLAGS_TIMEOUT_DECISION_MASK;
            return resolve_eager(decision == HOLD_TAP_FLAGS_TIMEOUT_DECISION_HOLD, ks, behavior->flags);
        }
#ifdef HOLD_TAP_QUICK_TAP_ENABLE
        if (*state == STATE_PRE_QUICK_TAP && delta >= behavior->quick_tap_ms) {
            TEMP_TAP(ks, 0);
            return HANDLE_RESULT_COMPLETED;
        }
#ifdef HOLD_TAP_QUICK_TAP_INTERRUPT_ENABLE
        if (*state == STATE_POST_QUICK_TAP && delta >= behavior->quick_tap_interrupt_ms) {
            ks->key_code &= KEY_CODE_TAP_MASK;
            return HANDLE_RESULT_MAPPED;
        }
#endif
#endif
    }

    switch (handle_event) {
    case HANDLE_EVENT_QUEUED:
#ifdef HOLD_TAP_GLOBAL_QUICK_TAP_ENABLE
        if (
            behavior->global_quick_tap_ms &&
            (get_timer() - get_last_tap_timestamp()) < behavior->global_quick_tap_ms
        ) {
#ifdef HOLD_TAP_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE_ENABLE
            if (behavior->flags & HOLD_TAP_FLAGS_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE) {
                TEMP_TAP(ks, 1);
                *state = STATE_IMMEDIATE_TAP;
                break;
            }
#endif
            ks->key_code &= KEY_CODE_TAP_MASK;
            return HANDLE_RESULT_MAPPED;
        }
#endif
        process_eager(ks, behavior->flags, EAGER_START);
        break;
    
    case HANDLE_EVENT_INCOMING_EVENT:
        {} // SDCC doesn't let me compile without this for some reason
        fak_key_event_t *ev_in = key_event_queue_bfront();
        __bit same_key_idx = ev_front->key_idx == ev_in->key_idx;

        switch (*state) {
        case STATE_DEFAULT:
            if (same_key_idx) {
                if (!ev_in->pressed) {
#ifdef HOLD_TAP_QUICK_TAP_ENABLE
                    // Process quick-tap if nothing was queued during tap-hold decision
                    if (behavior->quick_tap_ms && key_event_queue_get_size() == 1) {
                        if (!process_eager(ks, behavior->flags, EAGER_DECIDE_TAP)) {
                            TEMP_TAP(ks, 1);
                        }
                        *state = STATE_PRE_QUICK_TAP;
                        ev_front->timestamp = get_timer();
                        return HANDLE_RESULT_CONSUMED_EVENT;
                    }
#endif
                    if (process_eager(ks, behavior->flags, EAGER_DECIDE_TAP)) {
                        TEMP_TAP(ks, 0);
                    } else {
                        tap_non_future(ks->key_code & KEY_CODE_TAP_MASK);
                    }
                    return HANDLE_RESULT_COMPLETED | HANDLE_RESULT_CONSUMED_EVENT;
                }
            } else {
                uint8_t key_interrupt = get_key_interrupt(behavior, ev_in->key_idx);

                if (
                    (key_interrupt & HOLD_TAP_KEY_INTERRUPT_ENABLE)
                    && ev_in->pressed == ((key_interrupt & 4) >> 2)
                ) {
                    uint8_t decide_hold = key_interrupt & HOLD_TAP_KEY_INTERRUPT_DECIDE_HOLD;
                    return resolve_eager(decide_hold, ks, behavior->flags);
                }
            }
            break;

#ifdef HOLD_TAP_QUICK_TAP_ENABLE
        case STATE_PRE_QUICK_TAP:
            if (same_key_idx) {
#ifdef HOLD_TAP_QUICK_TAP_INTERRUPT_ENABLE
                if (behavior->quick_tap_interrupt_ms) {
                    *state = STATE_POST_QUICK_TAP;
                    ev_front->timestamp = get_timer();
                    return HANDLE_RESULT_CONSUMED_EVENT;
                }
#endif
                TEMP_TAP(ks, 0);
                ks->key_code &= KEY_CODE_TAP_MASK;
                return HANDLE_RESULT_MAPPED | HANDLE_RESULT_CONSUMED_EVENT;
            } else if (ev_in->pressed) {
                TEMP_TAP(ks, 0);
                return HANDLE_RESULT_COMPLETED;
            }
            break;

#ifdef HOLD_TAP_QUICK_TAP_INTERRUPT_ENABLE
        case STATE_POST_QUICK_TAP:
            TEMP_TAP(ks, 0);
            USB_EP1I_send_now();

            if (same_key_idx) {
                tap_non_future(ks->key_code & KEY_CODE_TAP_MASK);
                return HANDLE_RESULT_COMPLETED | HANDLE_RESULT_CONSUMED_EVENT;
            }
            // TODO: Deduplicate with above
            uint8_t key_interrupt = get_key_interrupt(behavior, ev_in->key_idx);

            if (
                (key_interrupt & HOLD_TAP_KEY_INTERRUPT_ENABLE)
                && ev_in->pressed == ((key_interrupt & 4) >> 2)
            ) {
                if (key_interrupt & HOLD_TAP_KEY_INTERRUPT_DECIDE_HOLD) {
                    ks->key_code &= KEY_CODE_HOLD_LAYER_IDX_MODS_MASK;
                } else {
                    ks->key_code &= KEY_CODE_TAP_MASK;
                }
                return HANDLE_RESULT_MAPPED;
            }
            break;
#endif
#endif

#if defined(HOLD_TAP_GLOBAL_QUICK_TAP_ENABLE) && defined(HOLD_TAP_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE_ENABLE)
        case STATE_IMMEDIATE_TAP:
            if (same_key_idx) {
                TEMP_TAP(ks, 0);
                *state = STATE_POST_IMMEDIATE_TAP;
                return HANDLE_RESULT_CONSUMED_EVENT;
            }

            ks->key_code &= KEY_CODE_TAP_MASK;
            return HANDLE_RESULT_MAPPED;
        
        case STATE_POST_IMMEDIATE_TAP:
            if (!same_key_idx) {
                return HANDLE_RESULT_COMPLETED;
            }

            *state = STATE_DEFAULT;
            return HANDLE_RESULT_CONSUMED_EVENT;
#endif
        }
    }

    return 0;
}
