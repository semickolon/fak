#ifndef __TIME_H__
#define __TIME_H__

void delay(uint16_t ms);
uint16_t get_timer();

void CLK_init();

void TMR0_interrupt();
void TMR0_init();

#endif
