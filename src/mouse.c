#include "mouse.h"
#include "usb.h"

void mouse_handle_key(uint16_t custom_code, uint8_t down) {
    if (custom_code < 8) {
        uint8_t buttons = USB_EP3I_read(0);
        buttons = (buttons & ~(1 << custom_code)) | (down << custom_code);
        return USB_EP3I_write(0, buttons);
    }

    switch (custom_code) {
    case 8:  USB_EP3I_write(1, down * 4);  break; // Right
    case 9:  USB_EP3I_write(1, -down * 4); break; // Left
    case 10: USB_EP3I_write(2, down * 4);  break; // Down
    case 11: USB_EP3I_write(2, -down * 4); break; // Up

    case 12: USB_EP3I_write(3, down);      break; // Wheel up
    case 13: USB_EP3I_write(3, -down);     break; // Wheel down
    }
}

void mouse_process() {
    USB_EP3I_send_now();
}
