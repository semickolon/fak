#ifndef __COMBO_H__
#define __COMBO_H__

#include <stdint.h>

#define COMBO_COUNT 3
#define COMBO_KEY_QUEUE_LEN 4
#define COMBO_MAX_KEY_COUNT 2
#define COMBO_KEY_IDX_START 5

typedef struct {
    uint8_t key_count;
    uint8_t timeout_ms;
    uint8_t key_indices[COMBO_MAX_KEY_COUNT];
} fak_combo_def_t;

void combo_push_key_event(uint8_t key_idx, uint8_t pressed);
void combo_handle();
void combo_init();

#endif // __COMBO_H__
