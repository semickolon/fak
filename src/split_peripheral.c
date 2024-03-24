#include "split_peripheral.h"
#include "ch55x.h"
#include "time.h"

uint8_t key_bits[SPLIT_KEY_COUNT_BYTES];
#if SPLIT_ENCODER_COUNT_BYTES > 0
uint8_t encoder_bits[SPLIT_ENCODER_COUNT_BYTES];
#endif

void key_state_inform(uint8_t key_idx, uint8_t down) {
    uint8_t shift = key_idx % 8;
    ES = 0;
    key_bits[key_idx / 8] = (key_bits[key_idx / 8] & ~(1 << shift)) | (down << shift);
    ES = 1;
}

#if SPLIT_ENCODER_COUNT_BYTES > 0
void encoder_scan(uint8_t encoder_idx, uint8_t reading) {
    uint8_t shift = (encoder_idx % 4) * 2;
    ES = 0;
    encoder_bits[encoder_idx / 4] = encoder_bits[encoder_idx / 4] & ~(0x03 << shift) | (reading << shift);
    ES = 1;
}
#endif

void keyboard_init() {
    for (uint8_t i = SPLIT_KEY_COUNT_BYTES; i;) {
        key_bits[--i] = 0;
    }

#if SPLIT_ENCODER_COUNT_BYTES > 0
    for (uint8_t i = SPLIT_ENCODER_COUNT_BYTES; i;) {
        encoder_bits[--i] = 0;
    }
#endif

    REN = 1;
    ES = 1;

    keyboard_init_user();
}

void keyboard_scan() {
    keyboard_scan_user();
}

void UART_interrupt() {
    if (!RI) return;
    uint8_t request = SBUF;
    RI = 0;
    REN = 0;

    if (request >= SPLIT_MSG_REQUEST_KEYS && request < SPLIT_MSG_REQUEST_KEYS + SPLIT_KEY_COUNT_BYTES) {
        SBUF = key_bits[request - SPLIT_MSG_REQUEST_KEYS];
    }
#if SPLIT_ENCODER_COUNT_BYTES > 0
    else if (request >= SPLIT_MSG_REQUEST_ENCODERS && request < SPLIT_MSG_REQUEST_ENCODERS + SPLIT_ENCODER_COUNT_BYTES) {
        SBUF = encoder_bits[request - SPLIT_MSG_REQUEST_ENCODERS];
    }
#endif
    else {
        SBUF = 0;
    }

    while (!TI);
    TI = 0;
    REN = 1;
}
