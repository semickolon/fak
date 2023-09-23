#include "key_event_queue.h"
#include "keymap.h"

typedef struct {
    fak_key_event_t q[KEY_EVENT_QUEUE_LEN];
    uint8_t size;
    uint8_t bsize;
    uint8_t state;
} fak_key_event_queue_t;

__xdata __at(XADDR_KEY_EVENT_QUEUE) fak_key_event_queue_t key_event_queue;

inline uint8_t key_event_queue_get_size() {
    return key_event_queue.size;
}

inline uint8_t key_event_queue_get_bsize() {
    return key_event_queue.bsize;
}

inline uint8_t* key_event_queue_state() {
    return &key_event_queue.state;
}

inline fak_key_event_t* key_event_queue_front() {
    return &key_event_queue.q[0];
}

inline fak_key_event_t* key_event_queue_bfront() {
    return &key_event_queue.q[key_event_queue.size];
}

void key_event_queue_push() {
    if (!key_event_queue.bsize) return;
    key_event_queue.size++;
    key_event_queue.bsize--;
}

void key_event_queue_pop() {
    if (!key_event_queue.size) return;

    for (uint8_t i = 0; i < (key_event_queue.size + key_event_queue.bsize - 1); i++) {
        key_event_queue.q[i] = key_event_queue.q[i + 1];
    }

    key_event_queue.bsize += key_event_queue.size - 1;
    key_event_queue.size = 0;
}

void key_event_queue_bpush(fak_key_event_t *ev) {
    key_event_queue.q[key_event_queue.size + key_event_queue.bsize] = *ev;
    key_event_queue.bsize++;
}

void key_event_queue_bpop() {
    if (!key_event_queue.bsize) return;

    for (uint8_t i = 0; i < key_event_queue.bsize - 1; i++) {
        key_event_queue.q[key_event_queue.size + i] = key_event_queue.q[key_event_queue.size + i + 1];
    }
    key_event_queue.bsize--;
}

void key_event_queue_breset() {
    key_event_queue.bsize += key_event_queue.size;
    key_event_queue.size = 0;
}

void key_event_queue_init() {
    key_event_queue.size = 0;
    key_event_queue.bsize = 0;
    key_event_queue.state = 0;
}
