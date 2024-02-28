#include "split_peripheral.h"
#include "ch55x.h"
#include "time.h"

uint8_t key_bits[SPLIT_KEY_COUNT_BYTES];

void key_state_inform(uint8_t key_idx, uint8_t down) {
    uint8_t shift = key_idx % 8;
    ES = 0;
    key_bits[key_idx / 8] = (key_bits[key_idx / 8] & ~(1 << shift)) | (down << shift);
    ES = 1;
}

void keyboard_init() {
    for (uint8_t i = SPLIT_KEY_COUNT_BYTES; i;) {
        key_bits[--i] = 0;
    }

    SM0 = 1;
    SM1 = 0;
    SM2 = 0;
    REN = 1;
    ES = 1;

    keyboard_init_user();
}

void keyboard_scan() {
    keyboard_scan_user();
}

void UART_interrupt() {
    if (!RI) return;
    RI = 0;
    REN = 0;
    TB8 = 0;

    for (uint8_t i = 0; i < SPLIT_KEY_COUNT_BYTES; i++) {
        SBUF = key_bits[i];
        while (!TI);
        TI = 0;
    }

    REN = 1;
}
