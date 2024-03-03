#include "split_central.h"
#include "ch55x.h"
#include "usb.h"
#include "time.h"
#include "keymap.h"
#include "bootloader.h"

#ifdef HOLD_TAP_ENABLE
#include "hold_tap.h"
#endif
#ifdef TAP_DANCE_ENABLE
#include "tap_dance.h"
#endif
#if COMBO_COUNT > 0
#include "combo.h"
#endif
#ifdef MOUSE_KEYS_ENABLE
#include "mouse.h"
#endif
#ifdef MACRO_KEYS_ENABLE
#include "macro.h"
#endif
#ifdef CAPS_WORD_ENABLE
#include "caps_word.h"
#endif

__xdata __at(XADDR_LAST_TAP_TIMESTAMP) uint16_t last_tap_timestamp = 0;
__xdata __at(XADDR_KEY_STATES) fak_key_state_t key_states[KEY_COUNT];

__xdata __at(XADDR_STRONG_MODS_REF_COUNT) uint8_t strong_mods_ref_count[8];

#ifdef STICKY_ENABLE
__xdata __at(XADDR_PENDING_STICKY_MODS) uint8_t pending_sticky_mods = 0;
__xdata __at(XADDR_APPLIED_STICKY_MODS) uint8_t applied_sticky_mods = 0;
__xdata __at(XADDR_APPLIED_STICKY_LAYER) uint8_t applied_sticky_layer = 0;
#endif

#ifdef REPEAT_KEY_ENABLE
__xdata __at(XADDR_REPEAT_KEY + 0) uint8_t pending_repeat_mods = 0;
__xdata __at(XADDR_REPEAT_KEY + 1) uint8_t pending_repeat_code = 0;
__xdata __at(XADDR_REPEAT_KEY + 2) uint8_t applied_repeat_mods = 0;
__xdata __at(XADDR_REPEAT_KEY + 3) uint8_t applied_repeat_code = 0;
#endif

#ifdef SPLIT_ENABLE
extern __code uint8_t split_periph_key_indices[SPLIT_PERIPH_KEY_COUNT];
#endif

uint16_t get_last_tap_timestamp() {
    return last_tap_timestamp;
}

uint8_t get_future_type(uint32_t key_code) {
#ifdef TAP_DANCE_ENABLE
    if (is_future_type_tap_dance(key_code)) return FUTURE_TYPE_TAP_DANCE;
#endif
#ifdef HOLD_TAP_ENABLE
    if (is_future_type_hold_tap(key_code)) return FUTURE_TYPE_HOLD_TAP;
#endif
#ifdef TRANS_LAYER_EXIT_ENABLE
    if (key_code == KEY_CODE_HOLD_TRANS_LAYER_EXIT || key_code == KEY_CODE_TAP_TRANS_LAYER_EXIT)
        return FUTURE_TYPE_TRANS_LAYER_EXIT;
#endif
    return FUTURE_TYPE_NONE;
}

static uint8_t key_check(uint8_t key_code) {
    uint8_t ret = 0;

    for (uint8_t i = 2; i < 8; i++) {
        uint8_t c = USB_EP1I_read(i);
        
        if (c == key_code && !(ret & 0x0F)) {
            ret |= i;
        }
        if (c == 0 && !(ret & 0xF0)) {
            ret |= (i << 4);
        }
    }

    return ret;
}

static void register_mods(uint8_t mods, uint8_t down) {
    uint8_t new_mods = 0;

    for (uint8_t i = 8; i;) {
        i--;
        
        if (mods & (1 << i)) {
            if (down) {
                strong_mods_ref_count[i] += 1;
            } else if (strong_mods_ref_count[i] > 0) {
                strong_mods_ref_count[i] -= 1;
            }
        }

        new_mods |= ((strong_mods_ref_count[i] > 0) << i);
    }

    uint8_t current_mods = USB_EP1I_read(0);

    if (current_mods != new_mods) {
        USB_EP1I_write(0, new_mods);
    }
}

static void write_weak_mods(uint8_t mods, uint8_t down) {
    uint8_t current_mods = USB_EP1I_read(0);

    uint8_t new_mods = 0;

    if (down) {
        new_mods = current_mods | mods;
    } else {
        for (uint8_t i = 8; i;) {
            i--;

            new_mods |= ((strong_mods_ref_count[i] > 0) << i);
        }
    }

    if (current_mods != new_mods) {
        USB_EP1I_write(0, new_mods);
    }
}

static void register_code(uint8_t key_code, uint8_t down) {
    uint8_t key_check_ret = key_check(key_code);
    uint8_t match_idx = key_check_ret & 0x0F;
    uint8_t empty_idx = (key_check_ret & 0xF0) >> 4;

    __bit write = (down && !match_idx && empty_idx) || (!down && match_idx);

    if (!write) {
        return;
    }

#ifdef STICKY_ENABLE
    if (!applied_sticky_mods && pending_sticky_mods && down) {
        applied_sticky_mods = pending_sticky_mods;
        pending_sticky_mods = 0;
        register_mods(applied_sticky_mods, 1);
    } else if (applied_sticky_mods) {
        register_mods(applied_sticky_mods, 0);
        applied_sticky_mods = 0;
    }
#endif

#ifdef REPEAT_KEY_ENABLE
    if (down) {
        pending_repeat_mods = USB_EP1I_read(0);
        pending_repeat_code = key_code;
    }
#endif

    USB_EP1I_write(
        down ? empty_idx : match_idx,
        down ? key_code  : 0
    );

    last_tap_timestamp = get_timer();
}

#ifdef TRANS_LAYER_EXIT_ENABLE
static uint8_t trans_layer_exit_handle(fak_key_state_t *ks) {
    uint8_t key_idx = key_event_queue_front()->key_idx;
    uint8_t layer_idx = get_trans_layer_exit_source_idx(
        key_idx,
        ks->key_code == KEY_CODE_HOLD_TRANS_LAYER_EXIT
    );

    if (layer_idx >= LAYER_COUNT)
        return HANDLE_RESULT_COMPLETED;

    layer_state_off(layer_idx);
    ks->key_code = get_real_key_code(key_idx);
    return HANDLE_RESULT_MAPPED;
}
#endif

static void subhandle(uint8_t handle_event) {
    fak_key_event_t *ev_front = key_event_queue_front();
    fak_key_state_t *ks = &key_states[ev_front->key_idx];
    uint8_t handle_result = HANDLE_RESULT_COMPLETED;

    if (!ev_front->mapped && handle_event == HANDLE_EVENT_QUEUED && ev_front->pressed) {
        ks->key_code = get_real_key_code(ev_front->key_idx);
#ifdef STICKY_ENABLE
        if (applied_sticky_layer > 0) {
            layer_state_off(applied_sticky_layer);
            applied_sticky_layer = 0;
        }
#endif
    }

    uint8_t future_type = get_future_type(ks->key_code);    

    if (future_type == FUTURE_TYPE_NONE) {
        handle_non_future(ks->key_code, ev_front->pressed);
        ks->status = (ks->status & ~KEY_STATUS_RESOLVED) | (ev_front->pressed << 2);
        handle_result = HANDLE_RESULT_COMPLETED;
    } else {
        int16_t delta = 0;

        if (handle_event == HANDLE_EVENT_PRE_SCAN) {
            delta = get_timer() - ev_front->timestamp;
        } else if (handle_event == HANDLE_EVENT_INCOMING_EVENT) {
            delta = key_event_queue_bfront()->timestamp - ev_front->timestamp;
        }

        switch (future_type) {
#ifdef TAP_DANCE_ENABLE
        case FUTURE_TYPE_TAP_DANCE:
            handle_result = tap_dance_handle_event(ks, handle_event, delta);
            break;
#endif
#ifdef HOLD_TAP_ENABLE
        case FUTURE_TYPE_HOLD_TAP:
            handle_result = hold_tap_handle_event(ks, handle_event, delta);
            break;
#endif
#ifdef TRANS_LAYER_EXIT_ENABLE
        case FUTURE_TYPE_TRANS_LAYER_EXIT:
            handle_result = trans_layer_exit_handle(ks);
            break;
#endif
        }
    }

    USB_EP1I_send_now();

    if (handle_event == HANDLE_EVENT_INCOMING_EVENT) {
        if (handle_result & HANDLE_RESULT_CONSUMED_EVENT) {
            key_event_queue_bpop();
        } else {
            key_event_queue_push();
        }
    }

    if (handle_result & (HANDLE_RESULT_COMPLETED | HANDLE_RESULT_MAPPED)) {
        *(key_event_queue_state()) = 0;
    }

    if (handle_result & HANDLE_RESULT_COMPLETED) {
        key_event_queue_pop();
    } else if (handle_result & HANDLE_RESULT_MAPPED) {
        ev_front->mapped = 1;
        key_event_queue_breset();
    }
}

static void handle_key_events() {
    if (key_event_queue_get_bsize() == 0 && key_event_queue_get_size()) {
        subhandle(HANDLE_EVENT_PRE_SCAN);
        return;
    }

    while (key_event_queue_get_bsize()) {
        if (key_event_queue_get_size()) {
            subhandle(HANDLE_EVENT_INCOMING_EVENT);
        } else {
            key_event_queue_push();
            subhandle(HANDLE_EVENT_QUEUED);
        }
    }
}

void push_key_event(uint8_t key_idx, uint8_t pressed) {
    fak_key_state_t *ks = &key_states[key_idx];

    if (!pressed && (ks->status & KEY_STATUS_RESOLVED)) {
        handle_non_future(ks->key_code, 0);
        ks->status &= ~KEY_STATUS_RESOLVED;
        ks->key_code = 0;
        return;
    }

    fak_key_event_t key_ev = {
        .pressed = pressed,
        .mapped = 0,
        .key_idx = key_idx,
        .timestamp = get_timer()
    };

    key_event_queue_bpush(&key_ev);
}

#ifdef REPEAT_KEY_ENABLE
void handle_repeat_key(uint8_t down) {
    if (down) {
        applied_repeat_mods = pending_repeat_mods;
        applied_repeat_code = pending_repeat_code;
    }

    register_mods(applied_repeat_mods, down);
    register_code(applied_repeat_code, down);
}
#endif

void handle_non_future(uint32_t key_code, uint8_t down) {
    if ((key_code & KEY_CODE_HOLD_NO_OP) != KEY_CODE_HOLD_NO_OP) {
#if LAYER_COUNT > 1
        uint8_t hold_layer = (key_code & KEY_CODE_HOLD_LAYER_IDX_MASK) >> 24;
        if (hold_layer) {
            if (down) {
                layer_state_on(hold_layer);
            } else {
                layer_state_off(hold_layer);
            }
        }
#endif
        uint8_t hold_mods = (key_code & KEY_CODE_HOLD_MODS_MASK) >> 16;
        if (hold_mods) {
            register_mods(hold_mods, down);
        }
    }

    uint8_t tap_mods = (key_code & KEY_CODE_TAP_MODS_MASK) >> 8;
    uint8_t tap_code = (key_code & KEY_CODE_TAP_CODE_MASK);
    uint8_t layer_idx = tap_code & 0x1F;
    uint8_t weak_mods = 0;

    switch (tap_code & 0xE0) {
#ifdef STICKY_ENABLE
    case 0xA0: // Sticky layer/mods
        if (!down) break;
        pending_sticky_mods |= tap_mods;

        if (layer_idx > 0 && is_layer_off(layer_idx)) {
            applied_sticky_layer = layer_idx;
            layer_state_on(applied_sticky_layer);
        }
        break;
#endif

#if LAYER_COUNT > 1
    case 0xC0: // Layer-tap action
        if (!down) break;

        switch (tap_mods) {
        case 0: // DF
            set_default_layer_idx(layer_idx);
            set_layer_state(0);
            break;
        case 1: // TG
            layer_state_toggle(layer_idx);
            break;
        case 2: // TO
            set_layer_state(1 << layer_idx);
            break;
        }
        break;
#endif
    
#ifdef CUSTOM_KEYS_ENABLE
    case 0xE0: // Custom keycode
        {}
        uint8_t custom_type = key_code & 0x07; // 3 bits
        uint16_t custom_code = (tap_mods << 2) | ((key_code & 0x18) >> 3); // 10 bits

        switch (custom_type) {
#ifdef FAK_KEYS_ENABLE
        case 0: // FAK-specific
            switch (custom_code) {
            case 0:
                sw_reset();
                break;
            case 1:
                bootloader();
                break;
#ifdef CAPS_WORD_ENABLE
            case 2:
                caps_word_on();
                break;
            case 3:
                caps_word_off();
                break;
            case 4:
                if (down) caps_word_toggle();
                break;
#endif
#ifdef REPEAT_KEY_ENABLE
            case 5:
                handle_repeat_key(down);
                break;
#endif
            }
            break;
#endif
#ifdef CONSUMER_KEYS_ENABLE
        case 1: // Consumer
            USB_EP2I_write_now(0, down ? custom_code : 0);
            break;
#endif
#ifdef USER_KEYS_ENABLE
        case 2: // User-defined
            break;
#endif
#ifdef MOUSE_KEYS_ENABLE
        case 3: // Mouse
            return mouse_handle_key(custom_code, down);
#endif
#ifdef MACRO_KEYS_ENABLE
        case 4: // Macro
            return macro_handle_key(custom_code, down);
#endif
        }
        break;
#endif

    default:

#ifdef CAPS_WORD_ENABLE
        if (down && tap_code && caps_word_active()) {
            uint8_t shift_pressed = (strong_mods_ref_count[1] > 0) | tap_mods & 0x02;
            if (caps_word_handle_key(tap_code, shift_pressed)) {
                weak_mods |= 0x02; // press shift key
            }
        }
#endif

        if (down && weak_mods) write_weak_mods(weak_mods, 1);
        if (tap_mods) register_mods(tap_mods, down);
        if (tap_code) register_code(tap_code, down);
        if (!down) write_weak_mods(weak_mods, 0);
    }
}

void tap_non_future(uint32_t key_code) {
    handle_non_future(key_code, 1);
    USB_EP1I_send_now();
    handle_non_future(key_code, 0);
    USB_EP1I_send_now();
}

void key_state_inform(uint8_t key_idx, uint8_t down) {
    fak_key_state_t *ks = &key_states[key_idx];
    uint8_t last_down = (ks->status & KEY_STATUS_DEBOUNCE) >> 1;
    
    if (last_down == down) {
        uint8_t last_pressed = ks->status & KEY_STATUS_DOWN;
        if (last_pressed == down) return;

        ks->status = ks->status & ~KEY_STATUS_DOWN | down;
#if COMBO_COUNT > 0
        combo_push_key_event(key_idx, down);
#else
        push_key_event(key_idx, down);
#endif
    } else {
        ks->status = ks->status & ~KEY_STATUS_DEBOUNCE | (down << 1);
    }
}

#ifdef SPLIT_ENABLE
static void split_periph_init() {
    SM0 = 1;
    SM1 = 0;
}

static void split_periph_scan() {
    SBUF = SPLIT_MSG_REQUEST_KEYS;
    TB8 = 0;
    while (!TI);
    TI = 0;
    REN = 1;

    uint8_t i = 0;
    uint8_t wait_cycles = 0;

    for (uint8_t b = 0; b < SPLIT_KEY_COUNT_BYTES; b++) {
        while (!RI) {
            if (++wait_cycles == 0) goto exit;
        }
        RI = 0;

        for (uint8_t bit = 0; bit < 8; bit++) {
            key_state_inform(split_periph_key_indices[i], (SBUF >> bit) & 1);
            if (++i == SPLIT_PERIPH_KEY_COUNT) break;
        }
    }

exit:
    REN = 0;
}
#endif

void keyboard_init() {
    for (uint8_t i = KEY_COUNT; i;) {
        key_states[--i].status = 0;
    }

    for (uint8_t i = 8; i;) {
        i--;
        strong_mods_ref_count[i] = 0;
    }

#if COMBO_COUNT > 0
    combo_init();
#endif
#ifdef SPLIT_ENABLE
    split_periph_init();
#endif
    key_event_queue_init();
    keyboard_init_user();
}

void keyboard_scan() {
    keyboard_scan_user();
#ifdef SPLIT_ENABLE
    split_periph_scan();
#endif
    delay(DEBOUNCE_MS);
#if COMBO_COUNT > 0
    combo_handle();
#endif
#ifdef MOUSE_KEYS_ENABLE
    mouse_process();
#endif
    handle_key_events();
}
