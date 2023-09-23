#include "ch552.h"
#include "time.h"

__idata volatile uint16_t timer_1ms;

void delay(uint16_t ms) {
    while (ms) {
        while ((TKEY_CTRL & bTKC_IF) == 0);
        while (TKEY_CTRL & bTKC_IF);
        ms--;
    }
}

uint16_t get_timer() {
    uint16_t ret;
    ET0 = 0;
    ret = timer_1ms;
    ET0 = 1;
    return ret;
}

static inline void enter_safe_mode() {
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
}

static inline void exit_safe_mode() {
    SAFE_MOD = 0x00;
}

void CLK_init() {
    // Internal clock @ 24MHz
    enter_safe_mode();
    CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_SEL) | 0b110;
    exit_safe_mode();
}

#pragma save
#pragma nooverlay
void TMR0_interrupt() {
    TL0 = 0x30;
    TH0 = 0xF8; // 65536 - 2000 = 63536
    timer_1ms++;
}
#pragma restore

void TMR0_init() {
    TL0 = 0x30;
    TH0 = 0xF8; // 65536 - 2000 = 63536
    TMOD = bT0_M0;

    timer_1ms = 0;

    ET0 = 1;
    TR0 = 1;
}
