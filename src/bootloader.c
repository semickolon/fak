#include "bootloader.h"
#include "ch552.h"
#include "time.h"

inline void bootloader() {
    USB_CTRL = 0;
    EA = 0;
    delay(1);
    ((void (*__data)(void)) BOOT_ADDR)();
}
