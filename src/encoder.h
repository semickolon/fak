#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <stdint.h>

#define ENC_READ(n) ((!ENC_A##n << 1) | !ENC_B##n)

void encoder_init();
void encoder_scan(uint8_t i, uint8_t reading);

#endif // __ENCODER_H__
