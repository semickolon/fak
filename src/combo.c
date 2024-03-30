#include "combo.h"
#include "time.h"
#include "split_central.h"

#define COMBO_STATE_IDLE      0
#define COMBO_STATE_TIMED_OUT 1
#define COMBO_STATE_ACTIVE    2

#define REF_COUNT_OWNED 255

#define COMBO_KEY_COUNT(combo_def) ((combo_def.flags & COMBO_FLAGS_KEY_COUNT_MASK) + 2)

typedef struct {
    uint8_t key_idx;
    uint8_t ref_count;
    uint16_t timestamp;
} fak_combo_key_queue_entry_t;

typedef struct {
    uint8_t size;
    fak_combo_key_queue_entry_t q[COMBO_KEY_QUEUE_LEN];
} fak_combo_key_queue_t;

extern __code fak_combo_def_t combo_defs[COMBO_COUNT];

__xdata __at(XADDR_COMBO_STATES) uint8_t combo_states[COMBO_COUNT]; // TODO: to bit array
__xdata __at(XADDR_COMBO_KEY_QUEUE) fak_combo_key_queue_t combo_key_queue;

static void combo_key_queue_remove(uint8_t idx) {
    for (uint8_t i = idx; i < combo_key_queue.size - 1; i++) {
        combo_key_queue.q[i] = combo_key_queue.q[i + 1];
    }
    combo_key_queue.size--;
}

void combo_push_key_event(uint8_t key_idx, uint8_t pressed) {
    if (pressed) {
#ifdef COMBO_REQUIRE_PRIOR_IDLE_MS_ENABLE
        uint16_t last_tap_delta = get_timer() - get_last_tap_timestamp();
#endif

        // Add to combo key queue if key is in any (handle-able) combo
        for (uint8_t i = COMBO_COUNT; i;) {
            i--;

#ifdef COMBO_REQUIRE_PRIOR_IDLE_MS_ENABLE
            // Do not handle combo if it requires prior idle time
            uint16_t require_prior_idle_ms = combo_defs[i].require_prior_idle_ms;

            if (require_prior_idle_ms && require_prior_idle_ms > last_tap_delta) {
                continue;
            }
#endif

            for (uint8_t j = COMBO_KEY_COUNT(combo_defs[i]); j;) {
                if (combo_defs[i].key_indices[--j] != key_idx)
                    continue;
                
                fak_combo_key_queue_entry_t e = { 
                    .key_idx = key_idx,
                    .ref_count = 0,
                    .timestamp = get_timer(),
                };

                combo_key_queue.q[combo_key_queue.size++] = e;
                return combo_handle(0);
            }
        }

        // Otherwise, we have a non-combo key
        // Press all unowned combo keys, removing them all from the queue
        for (uint8_t i = 0; i < combo_key_queue.size; i++) {
            if (combo_key_queue.q[i].ref_count != REF_COUNT_OWNED) {
                push_key_event(combo_key_queue.q[i].key_idx, 1);
                combo_key_queue_remove(i);
            }
        }

        // Then press the non-combo key itself
        return push_key_event(key_idx, 1);
    }

    // On release, we check if this key is in the queue
    for (uint8_t i = 0; i < combo_key_queue.size; i++) {
        if (key_idx != combo_key_queue.q[i].key_idx)
            continue;
        
        // If it's owned, we let the combo processor handle it
        __bit owned = combo_key_queue.q[i].ref_count == REF_COUNT_OWNED;

        if (owned) {
            combo_key_queue_remove(i);
            return combo_handle(0);
        } else {
            return combo_handle(1);
            // combo_k{ey_queue_remove(i);
            // return }combo_handle(0);
        }

        // Otherwise, we press all unowned combo keys *up until* the combo key removed just now
        // All these keys are also removed from the queue
        // This is done to preserve the keypress order
        uint8_t k = 0;

        for (uint8_t j = 0; j < i; j++) {
            if (combo_key_queue.q[j].ref_count == REF_COUNT_OWNED) {
                k++;
                continue;
            }

            push_key_event(combo_key_queue.q[k].key_idx, 1);
            combo_key_queue_remove(k);
        }

        // Tap the combo key removed just now
        push_key_event(key_idx, 1);
        break;
    }
    
    push_key_event(key_idx, 0);
}

void combo_handle(uint8_t ignore_parents) {
    // Reset ref counters except owned
    for (uint8_t i = combo_key_queue.size; i;) {
        uint8_t *ref_count = &combo_key_queue.q[--i].ref_count;
        if (*ref_count != REF_COUNT_OWNED) *ref_count = 0;
    }

    for (uint8_t i = 0; i < COMBO_COUNT; i++) {
        uint8_t pressed_keys = 0;
        uint8_t combo_key_count = COMBO_KEY_COUNT(combo_defs[i]);

        uint16_t last_timestamp_ms = 0;
        uint16_t extent_ms = 0;

        #define combo_def (combo_defs[i])
        #define combo_state (combo_states[i])
        #define all_pressed_keys ((1 << combo_key_count) - 1)

        __bit combo_keys_all_pressed = 0;
        
        // Enumerate pressed keys
        for (uint8_t k = 0; k < combo_key_queue.size; k++) {
            for (uint8_t j = 0; j < combo_key_count; j++) {
                if (combo_def.key_indices[j] != combo_key_queue.q[k].key_idx)
                    continue;
                
                // Bail out if we see an owned key, which means this combo is impossible to trigger for now
                // This only applies to inactive combos
                if (combo_state != COMBO_STATE_ACTIVE && combo_key_queue.q[k].ref_count == REF_COUNT_OWNED) {
                    goto exit_outer;
                }

                if (pressed_keys) {
                    extent_ms += combo_key_queue.q[k].timestamp - last_timestamp_ms;
                }

                last_timestamp_ms = combo_key_queue.q[k].timestamp;
                pressed_keys |= (1 << j);
                break;
            }

            if (pressed_keys == all_pressed_keys) {
                combo_keys_all_pressed = 1;
                break;
            }
        }

        if (combo_state != COMBO_STATE_ACTIVE && pressed_keys == 0) {
            combo_state = COMBO_STATE_IDLE;
            continue;
        }

        if (combo_state == COMBO_STATE_IDLE) {
            if (!combo_keys_all_pressed) {
                // For combo keys that aren't considered pressed, if any of them are actually down,
                // that means this isn't currently handled by combos and cannot be triggered by us for now.
                for (uint8_t j = 0; j < combo_key_count; j++) {
                    if ((pressed_keys & (1 << j)) == 0 && is_key_down(combo_def.key_indices[j])) {
                        goto exit_outer;
                    }
                }

                extent_ms += get_timer() - last_timestamp_ms;
            }

            if (extent_ms >= combo_def.timeout_ms) {
                combo_state = COMBO_STATE_TIMED_OUT;
                continue;
            }
        }

#ifdef COMBO_FULLY_OVERLAPPING
        __bit bruh = 0;

        if (ignore_parents && combo_keys_all_pressed) {
            ignore_parents = 0;
            bruh = 1;
        } else {
            // If parents exist, only process this combo if all parents have timed out
            for (uint8_t j = 0; j < combo_def.parent_count; j++) {
                if (combo_states[combo_def.parents[j]] != COMBO_STATE_TIMED_OUT) {
                    goto exit_outer;
                }
            }
        }
#endif

        if (combo_state == COMBO_STATE_IDLE) {
            // Populate ref counters
            for (uint8_t j = 0; j < combo_key_count; j++) {
                if (!(pressed_keys & (1 << j)))
                    continue;
                
                for (uint8_t k = 0; k < combo_key_queue.size; k++) {
                    if (combo_def.key_indices[j] != combo_key_queue.q[k].key_idx)
                        continue;
                    
                    if (combo_keys_all_pressed) {
                        combo_key_queue.q[k].ref_count = REF_COUNT_OWNED;
                    } else {
                        combo_key_queue.q[k].ref_count++;
                    } 

                    break;
                }
            }

            if (combo_keys_all_pressed) {
                combo_state = COMBO_STATE_ACTIVE;
                push_key_event(combo_def.key_idx_mapping, 1);

                if (bruh) {
                    combo_state = COMBO_STATE_IDLE;
                    push_key_event(combo_def.key_idx_mapping, 1);
                }
            }
        } else if (combo_state == COMBO_STATE_ACTIVE) {
            __bit release = 0;

            if (combo_def.flags & COMBO_FLAGS_SLOW_RELEASE_MASK) {
                release = pressed_keys == 0;
            } else {
                release = !combo_keys_all_pressed;
            }

            if (release) {
                combo_state = COMBO_STATE_IDLE;
                push_key_event(combo_def.key_idx_mapping, 0);
            }
        }
exit_outer:
    }

    // If no combo cares about a key anymore, press it then remove from queue
    for (uint8_t i = 0; i < combo_key_queue.size;) {
        uint8_t ref_count = combo_key_queue.q[i].ref_count;

        if (ref_count == REF_COUNT_OWNED) {
            i++;
            continue;
        } else if (ref_count != 0) {
            // We only do this until we see a key that combos still do care about
            // This is done to preserve the keypress order
            break;
        }
        
        push_key_event(combo_key_queue.q[i].key_idx, 1);
        combo_key_queue_remove(i);
    }
}

void combo_init() {
    for (uint8_t i = COMBO_COUNT; i;) {
        combo_states[--i] = COMBO_STATE_IDLE;
    }

    combo_key_queue.size = 0;
}
