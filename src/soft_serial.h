#ifndef __SOFT_SERIAL_H__
#define __SOFT_SERIAL_H__

#include <stdint.h>

__xdata __at(0x3FC) uint8_t soft_serial_sbuf;
__xdata __at(0x3FD) uint8_t soft_serial_did_not_respond;

void soft_serial_init();

void soft_serial_recv();
void soft_serial_send();

#endif // __SOFT_SERIAL_H__
