#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <stdint.h>

typedef struct {
    uint8_t resolution;
    uint8_t key_idx_cw;
    uint8_t key_idx_ccw;
} fak_encoder_def_t;

void encoder_init();
void encoder_scan(uint8_t i, uint8_t reading);

#endif // __ENCODER_H__
