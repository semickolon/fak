#include "split_peripheral.h"
#include "ch55x.h"
#include "time.h"
#include "bootloader.h"

#ifdef SPLIT_SOFT_SERIAL_PIN
#include "soft_serial.h"
#endif

__bit bootmagic_flag = 1;

uint8_t key_bits[SPLIT_KEY_COUNT_BYTES];
#if SPLIT_ENCODER_COUNT_BYTES > 0
uint8_t encoder_bits[SPLIT_ENCODER_COUNT_BYTES];
#endif

static uint8_t response_to(uint8_t request) {
    if (request >= SPLIT_MSG_REQUEST_KEYS && request < SPLIT_MSG_REQUEST_KEYS + SPLIT_KEY_COUNT_BYTES) {
        return key_bits[request - SPLIT_MSG_REQUEST_KEYS];
    }
#if SPLIT_ENCODER_COUNT_BYTES > 0
    else if (request >= SPLIT_MSG_REQUEST_ENCODERS && request < SPLIT_MSG_REQUEST_ENCODERS + SPLIT_ENCODER_COUNT_BYTES) {
        return encoder_bits[request - SPLIT_MSG_REQUEST_ENCODERS];
    }
#endif
    return 0;
}

#ifdef SPLIT_SOFT_SERIAL_PIN
uint8_t try_respond_to_soft_serial_request() {
    EA = 0;
    soft_serial_recv();
    
    if (!soft_serial_did_not_respond) {
        soft_serial_sbuf = response_to(soft_serial_sbuf);
        soft_serial_send();
    }

    EA = 1;
    return !soft_serial_did_not_respond;
}
#endif

void key_state_inform(uint8_t key_idx, uint8_t down) {
    if (key_idx == 0 && bootmagic_flag) {
        if (down)
            bootloader();

        bootmagic_flag = 0;
    }

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

#ifndef SPLIT_SOFT_SERIAL_PIN
    REN = 1;
    ES = 1;
#endif

    keyboard_init_user();
}

void keyboard_scan() {
    keyboard_scan_user();

#ifdef SPLIT_SOFT_SERIAL_PIN
    // Soft serial isn't interrupt-driven so we generously make way for it instead of continuously scanning
    __bit responding = 0;

    for (uint8_t i = 1000; i; i--) {
        if (try_respond_to_soft_serial_request()) {
            responding = 1;
        } else if (responding) {
            break;
        }
    }
#endif
}

#ifndef SPLIT_SOFT_SERIAL_PIN
void UART0_interrupt() {
    if (!RI) return;
    RI = 0;
    REN = 0;

    SBUF = response_to(SBUF);
    while (!TI);
    TI = 0;
    REN = 1;
}
#endif
