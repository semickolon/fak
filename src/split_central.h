#ifndef __SPLIT_CENTRAL_H__
#define __SPLIT_CENTRAL_H__

#include "key_event_queue.h"
#include "ch55x.h"

#include <stdint.h>

#define KEY_STATUS_DOWN 0x01
#define KEY_STATUS_DEBOUNCE 0x02
#define KEY_STATUS_RESOLVED 0x04

#define HANDLE_RESULT_MAPPED 0x01
#define HANDLE_RESULT_COMPLETED 0x02
#define HANDLE_RESULT_CONSUMED_EVENT 0x04

#define HANDLE_EVENT_QUEUED 0
#define HANDLE_EVENT_PRE_SCAN 1
#define HANDLE_EVENT_INCOMING_EVENT 2

#define FUTURE_TYPE_NONE 0
#define FUTURE_TYPE_HOLD_TAP 1
#define FUTURE_TYPE_TAP_DANCE 2
#define FUTURE_TYPE_TRANS_LAYER_EXIT 3

typedef struct {
    uint8_t status;
    uint32_t key_code;
} fak_key_state_t;

typedef struct {
    uint8_t type;
    union {
        fak_key_event_t* ev_in;
    };
} fak_handle_event_t;

void push_key_event(uint8_t key_idx, uint8_t pressed);
void handle_non_future(uint32_t key_code, uint8_t down);
void tap_non_future(uint32_t key_code);
uint32_t get_real_key_code(uint8_t key_idx);
uint8_t get_future_type(uint32_t key_code);
uint16_t get_last_tap_timestamp();
void key_state_inform(uint8_t key_idx, uint8_t down);

void keyboard_init();
void keyboard_scan();

#endif // __SPLIT_CENTRAL_H__
