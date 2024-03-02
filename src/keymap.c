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
#define IS_HOLD_TRANS (hold == 0xFFFF)
#define IS_TAP_TRANS (tap == 0xFFFF)
    uint16_t hold = 0xFFFF;
    uint16_t tap = 0xFFFF;
    uint8_t layer_idx = LAYER_COUNT - 1;

    do {
        if (!is_layer_on(layer_idx))
            continue;
        
        uint32_t key_code = key_map[layer_idx][key_idx];

        // Bail out if this keycode is not a hold-tap (e.g. tap dance)
        //  and if either hold or tap, but not both, is still transparent.
        if ((key_code >> 28) == 0xE && (IS_HOLD_TRANS ^ IS_TAP_TRANS)) {
            return 0;
        }

        if (IS_HOLD_TRANS) hold = key_code >> 16;
        if (IS_TAP_TRANS)  tap = key_code & 0xFFFF;
    } while (layer_idx-- && (IS_HOLD_TRANS || IS_TAP_TRANS));

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

static void on_layer_state_change() {
#if CONDITIONAL_LAYER_COUNT > 0
    for (uint8_t i = 0; i < CONDITIONAL_LAYER_COUNT; i++) {
        __code fak_conditional_layer_def_t *cl = &conditional_layers[i];

        if (((layer_state | persistent_layer_state) & cl->if_layers) == cl->if_layers) {
            layer_state |= (1 << cl->then_layer);
        } else {
            layer_state &= ~(1 << cl->then_layer);
        }
    }
#endif
}

void set_default_layer_idx(uint8_t layer_idx) {
    set_persistent_layer_state(1 << layer_idx);
}

void set_layer_state(fak_layer_state_t state) {
    layer_state = state;
    on_layer_state_change();
}

void layer_state_on(uint8_t layer_idx) {
    layer_state |= (1 << layer_idx);
    on_layer_state_change();
}

void layer_state_off(uint8_t layer_idx) {
    layer_state &= ~(1 << layer_idx);
    on_layer_state_change();
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
    on_layer_state_change();
}

void persistent_layer_state_on(uint8_t layer_idx) {
    persistent_layer_state |= (1 << layer_idx);
    on_layer_state_change();
}

void persistent_layer_state_off(uint8_t layer_idx) {
    persistent_layer_state &= ~(1 << layer_idx);
    on_layer_state_change();
}

uint8_t is_layer_on(uint8_t layer_idx) {
    return ((layer_state | persistent_layer_state) & (1 << layer_idx)) != 0;
}

uint8_t is_layer_off(uint8_t layer_idx) {
    return ((layer_state | persistent_layer_state) & (1 << layer_idx)) == 0;
}

#ifdef TRANS_LAYER_EXIT_ENABLE
uint8_t get_trans_layer_exit_source_idx(uint8_t key_idx, uint8_t hold) {
    uint32_t mask = hold ? KEY_CODE_HOLD_LAYER_IDX_MODS_MASK : KEY_CODE_TAP_MASK;
    uint32_t code = hold ? KEY_CODE_HOLD_TRANS_LAYER_EXIT : KEY_CODE_TAP_TRANS_LAYER_EXIT;
    
    uint8_t layer_idx = LAYER_COUNT;

    while (layer_idx--) {
        // Trans layer exit only works for non-persistent activated layers
        if ((layer_state & (1 << layer_idx)) && (key_map[layer_idx][key_idx] & mask) == code)
            return layer_idx;
    }

    return 255;
}
#endif

#endif
