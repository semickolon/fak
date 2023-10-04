#ifndef __USB_H__
#define __USB_H__

#include <stdint.h>

uint8_t USB_EP1I_read(uint8_t idx);
void USB_EP1I_write(uint8_t idx, uint8_t value);
inline void USB_EP1I_ready_send();
inline void USB_EP1I_send_now();

void USB_EP2I_write_now(uint8_t idx, uint16_t value);

void USB_interrupt();
void USB_init();

#endif // __USB_H__
