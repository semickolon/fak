#include "tap_dance.h"
#include "keymap.h"

__xdata __at(XADDR_TAP_COUNT) uint8_t tap_count = 1;

uint8_t is_future_type_tap_dance(uint32_t key_code) {
    return (key_code >> 24) == 0xE0;
}

uint8_t tap_dance_handle_event(fak_key_state_t *ks, uint8_t handle_ev, int16_t delta) {
    fak_key_event_t *ev_front = key_event_queue_front();

    if (handle_ev == HANDLE_EVENT_QUEUED && !ev_front->pressed) {
        return HANDLE_RESULT_COMPLETED;
    }

    uint8_t max_taps = (ks->key_code >> 20) & 0xF;
    uint16_t tapping_term_ms = (ks->key_code >> 8) & 0xFFF;

    if (tap_count < max_taps && delta < tapping_term_ms) {
        if (handle_ev != HANDLE_EVENT_INCOMING_EVENT) {
            return 0;
        }

        fak_key_event_t *ev_in = key_event_queue_bfront();

        if (ev_front->key_idx == ev_in->key_idx) {
            if (!ev_in->pressed) return 0;
            tap_count++;
            return HANDLE_RESULT_COMPLETED;
        }
    }

    uint8_t binding_start = ks->key_code & 0xFF;
    ks->key_code = tap_dance_bindings[binding_start + tap_count - 1];
    tap_count = 1;
    
    return HANDLE_RESULT_MAPPED;
}
