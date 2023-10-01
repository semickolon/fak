#include "ch552.h"
#include "keyboard.h"
#include "time.h"

#ifdef SPLIT_SIDE_CENTRAL
#include "usb.h"

void USB_interrupt();
void USB_ISR() __interrupt(INT_NO_USB) {
    USB_interrupt();
}
#endif

void TMR0_interrupt();
void TMR0_ISR() __interrupt(INT_NO_TMR0) {
    TMR0_interrupt();
}

#if defined(SPLIT_ENABLE) && defined(SPLIT_SIDE_PERIPHERAL)
void UART_interrupt();
void UART_ISR() __interrupt(INT_NO_UART0) {
    UART_interrupt();
}
#endif

void main() {
    CLK_init();
#ifdef SPLIT_SIDE_CENTRAL
    TMR0_init();
    USB_init();
#endif
    keyboard_init();

    EA = 1;

    while (1) {
        keyboard_scan();
    }
}
