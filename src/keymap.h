#ifndef __KEYMAP_H__
#define __KEYMAP_H__

#include <stdint.h>

#define KEY_CODE_HOLD_MASK 0xFFFF0000
#define KEY_CODE_HOLD_BEHAVIOR_MASK 0xE0000000
#define KEY_CODE_HOLD_LAYER_IDX_MASK 0x1F000000
#define KEY_CODE_HOLD_MODS_MASK 0x00FF0000
#define KEY_CODE_HOLD_LAYER_IDX_MODS_MASK 0x1FFF0000

#define KEY_CODE_TAP_MASK 0xFFFF
#define KEY_CODE_TAP_MODS_MASK 0xFF00
#define KEY_CODE_TAP_CODE_MASK 0xFF

#define KEY_CODE_HOLD_NO_OP 0x1FFF0000
#define KEY_CODE_HOLD_TRANS_LAYER_EXIT 0x1FFE0000
#define KEY_CODE_TAP_TRANS_LAYER_EXIT  0x0000FFFE

#if LAYER_COUNT == 1
extern __code uint32_t key_map[KEY_COUNT];

#else

#if LAYER_COUNT <= 8
typedef uint8_t fak_layer_state_t;
#elif LAYER_COUNT <= 16
typedef uint16_t fak_layer_state_t;
#else
typedef uint32_t fak_layer_state_t;
#endif

extern __code uint32_t key_map[LAYER_COUNT][KEY_COUNT];

#if CONDITIONAL_LAYER_COUNT > 0
typedef struct {
    uint8_t then_layer;
    fak_layer_state_t if_layers;
} fak_conditional_layer_def_t;

extern __code fak_conditional_layer_def_t conditional_layers[CONDITIONAL_LAYER_COUNT];
#endif

uint8_t get_highest_layer_idx();
uint8_t get_default_layer_idx();
void set_default_layer_idx(uint8_t layer_idx);

void set_layer_state(fak_layer_state_t state);
void layer_state_on(uint8_t layer_idx);
void layer_state_off(uint8_t layer_idx);
void layer_state_toggle(uint8_t layer_idx);

void set_persistent_layer_state(fak_layer_state_t state);
void persistent_layer_state_on(uint8_t layer_idx);
void persistent_layer_state_off(uint8_t layer_idx);

uint8_t is_layer_on(uint8_t layer_idx);
uint8_t is_layer_off(uint8_t layer_idx);

#ifdef TRANS_LAYER_EXIT_ENABLE
uint8_t get_trans_layer_exit_source_idx(uint8_t key_idx, uint8_t hold);
#endif

#endif

uint32_t get_real_key_code(uint8_t key_idx);

#endif // __KEYMAP_H__
