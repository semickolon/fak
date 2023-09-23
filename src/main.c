#include "ch552.h"
#include "time.h"
#include "usb.h"
#include "keyboard.h"

void USB_interrupt();
void USB_ISR() __interrupt(INT_NO_USB) {
    USB_interrupt();
}

void TMR0_interrupt();
void TMR0_ISR() __interrupt(INT_NO_TMR0) {
    TMR0_interrupt();
}

void main() {
    CLK_init();
    TMR0_init();
    USB_init();
    keyboard_init();

    EA = 1;

    while (1) {
        keyboard_scan();
    }
}
