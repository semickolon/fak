#include "encoder.h"
#include "keyboard.h"

__xdata __at(XADDR_ENCODER_STEPS) int8_t encoder_steps[ENCODER_COUNT];
__xdata __at(XADDR_ENCODER_LAST_NUM) uint8_t encoder_last_num[(ENCODER_COUNT + 3) / 4];

extern __code fak_encoder_def_t encoder_defs[ENCODER_COUNT];

void encoder_init() {
    for (uint8_t i = ENCODER_COUNT; i;) {
        encoder_steps[--i] = 0;
    }

    for (uint8_t i = sizeof(encoder_last_num); i;) {
        encoder_last_num[--i] = 0;
    }
}

void encoder_scan(uint8_t i, uint8_t reading) {
    int8_t direction = 0;

    {
        uint8_t bit_shift = (i % 4) * 2;
        uint8_t num = reading > 1 ? 5 - reading : reading; // 2 => 3, 3 => 2
        uint8_t last_num = (encoder_last_num[i / 4] >> bit_shift) & 0x03;

        if (last_num != num) {
            int8_t delta = last_num - num;
            encoder_last_num[i / 4] = encoder_last_num[i / 4] & ~(0x03 << bit_shift) | (num << bit_shift);

            if (delta == -1 || delta == 3) {
                direction = 1; // CW
            } else if (delta == 1 || delta == -3) {
                direction = -1; // CCW
            }
        }
    }

    if (direction == 0)
        return;

    int8_t steps = encoder_steps[i];

    if (steps < 0 != direction < 0) {
        steps = direction;
    } else {
        steps += direction;
    }

    if (steps != 0 && steps % encoder_defs[i].resolution == 0) {
        uint8_t key_idx = steps > 0 ? encoder_defs[i].key_idx_ccw : encoder_defs[i].key_idx_cw;
        steps = 0;

        if (key_idx) {
            push_key_event(key_idx, 1);
            push_key_event(key_idx, 0);
        }
    }

    encoder_steps[i] = steps;
}
