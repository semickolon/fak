#include "bootloader.h"
#include "ch55x.h"
#include "time.h"

inline void bootloader() {
    USB_CTRL = 0;
    EA = 0;
    delay(1);
    ((void (*__data)(void)) BOOT_ADDR)();
}

inline void sw_reset() {
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG |= bSW_RESET;
}
