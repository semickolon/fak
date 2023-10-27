#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <stdint.h>

void mouse_handle_key(uint16_t custom_code, uint8_t down);
void mouse_process();

#endif // __MOUSE_H__
