#ifndef __MACRO_H__
#define __MACRO_H__

#include <stdint.h>

#define MACRO_INST_HALT               0
#define MACRO_INST_PRESS              1
#define MACRO_INST_RELEASE            2
#define MACRO_INST_TAP                3
#define MACRO_INST_WAIT               4
#define MACRO_INST_PAUSE_FOR_RELEASE  5

typedef struct {
    uint8_t inst;
#if MACRO_STEP_ARG_COUNT > 256
    uint16_t arg_idx;
#else
    uint8_t arg_idx;
#endif
} fak_macro_step_t;

void macro_handle_key(uint16_t custom_code, uint8_t down);

extern __code fak_macro_step_t macro_steps[];
extern __code uint32_t macro_step_args[];

#endif // __MACRO_H__
