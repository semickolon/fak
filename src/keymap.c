#include "keymap.h"

#if LAYER_COUNT > 1
__xdata __at(XADDR_LAYER_STATE) fak_layer_state_t layer_state = 0;
__xdata __at(XADDR_PERSISTENT_LAYER_STATE) fak_layer_state_t persistent_layer_state = 1;
#endif

uint32_t get_real_key_code(uint8_t key_idx) {
#if LAYER_COUNT == 1
    return key_map[key_idx];
#else
#ifdef LAYER_TRANSPARENCY_ENABLE
    uint16_t hold = 0xFFFF;
    uint16_t tap = 0xFFFF;
    uint8_t layer_idx = LAYER_COUNT - 1;

    do {
        if (is_layer_on(layer_idx)) {
            uint32_t key_code = key_map[layer_idx][key_idx];
            if (hold == 0xFFFF) hold = key_code >> 16;
            if (tap == 0xFFFF)  tap = key_code & 0xFFFF;
        }
    } while (layer_idx-- && (hold == 0xFFFF || tap == 0xFFFF));

    if (hold == 0xFFFF) hold = 0;
    if (tap == 0xFFFF)  tap = 0;
    return ((uint32_t) hold << 16) | tap;
#else
    return key_map[get_highest_layer_idx()][key_idx];
#endif
#endif
}

#if LAYER_COUNT > 1

uint8_t get_highest_layer_idx() {
    for (uint8_t layer_idx = LAYER_COUNT - 1; layer_idx; layer_idx--) {
        if (is_layer_on(layer_idx))
            return layer_idx;
    }
    return 0;
}

uint8_t get_default_layer_idx() {
    for (uint8_t layer_idx = 0; layer_idx < LAYER_COUNT; layer_idx++) {
        if (persistent_layer_state & (1 << layer_idx))
            return layer_idx;
    }
    return 0;
}

void set_default_layer_idx(uint8_t layer_idx) {
    set_persistent_layer_state(1 << layer_idx);
}

void set_layer_state(fak_layer_state_t state) {
    layer_state = state;
}

void layer_state_on(uint8_t layer_idx) {
    layer_state |= (1 << layer_idx);
}

void layer_state_off(uint8_t layer_idx) {
    layer_state &= ~(1 << layer_idx);
}

void layer_state_toggle(uint8_t layer_idx) {
    if (is_layer_off(layer_idx)) {
        layer_state_on(layer_idx);
    } else {
        layer_state_off(layer_idx);
    }
}

void set_persistent_layer_state(fak_layer_state_t state) {
    persistent_layer_state = state;
}

void persistent_layer_state_on(uint8_t layer_idx) {
    persistent_layer_state |= (1 << layer_idx);
}

void persistent_layer_state_off(uint8_t layer_idx) {
    persistent_layer_state &= ~(1 << layer_idx);
}

uint8_t is_layer_on(uint8_t layer_idx) {
    return ((layer_state | persistent_layer_state) & (1 << layer_idx)) != 0;
}

uint8_t is_layer_off(uint8_t layer_idx) {
    return ((layer_state | persistent_layer_state) & (1 << layer_idx)) == 0;
}

#endif
