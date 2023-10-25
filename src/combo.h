#ifndef __COMBO_H__
#define __COMBO_H__

#include <stdint.h>

#define COMBO_FLAGS_KEY_COUNT_MASK 0x07
#define COMBO_FLAGS_SLOW_RELEASE_MASK 0x08

typedef struct {
    uint8_t flags;
    uint8_t timeout_ms;
#ifdef COMBO_REQUIRE_PRIOR_IDLE_MS_ENABLE
    uint16_t require_prior_idle_ms;
#endif
    uint8_t key_indices[COMBO_MAX_KEY_COUNT];
} fak_combo_def_t;

void combo_push_key_event(uint8_t key_idx, uint8_t pressed);
void combo_handle();
void combo_init();

#endif // __COMBO_H__
