
#ifndef __BOARD_H
#define __BOARD_H

volatile struct flag {
	uint8_t button : 1;
} flag;

void LED_Config ( void );
void PushButton_Config ( void );
void ADC_GetPotValue ( uint16_t * value );

#endif // __BOARD_H
