#include "mouse.h"
#include "usb.h"
#include "time.h"

__xdata __at(XADDR_MOUSE_SCROLL_DIRECTION) int8_t scroll_direction = 0;
__xdata __at(XADDR_MOUSE_SCROLL_AT_TIME) uint16_t scroll_at_time = 0;

void mouse_handle_key(uint16_t custom_code, uint8_t down) {
    if (custom_code < 8) {
        uint8_t buttons = USB_EP3I_read(0);
        buttons = (buttons & ~(1 << custom_code)) | (down << custom_code);
        return USB_EP3I_write(0, buttons);
    }

    switch (custom_code) {
    case 8:  USB_EP3I_write(1, down * MOUSE_MOVE_SPEED);  break; // Right
    case 9:  USB_EP3I_write(1, -down * MOUSE_MOVE_SPEED); break; // Left
    case 10: USB_EP3I_write(2, down * MOUSE_MOVE_SPEED);  break; // Down
    case 11: USB_EP3I_write(2, -down * MOUSE_MOVE_SPEED); break; // Up
    case 12: // Wheel up
    case 13: // Wheel down
        scroll_direction = custom_code == 12 ? down : -down;
        scroll_at_time = get_timer();
        break;
    }
}

void mouse_process() {
    if (scroll_direction != 0) {
        if (get_timer() >= scroll_at_time) {
            USB_EP3I_write(3, scroll_direction);
            USB_EP3I_send_now();
            USB_EP3I_write(3, 0);
            scroll_at_time = get_timer() + MOUSE_SCROLL_INTERVAL_MS;
        }
    } else {
        USB_EP3I_write(3, 0);
    }

    USB_EP3I_send_now();
}
