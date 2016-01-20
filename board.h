
#ifndef __BOARD_H
#define __BOARD_H

volatile struct flag {
	uint8_t button			: 1;
	uint8_t consigneUpdate	: 1;
	uint8_t mainProcess		: 1;
	uint8_t sendData		: 1;
	uint16_t				: 12;
} flag;

void LED_Config ( void );
void PushButton_Config ( void );
void ADC_GetPotValue ( uint16_t * value );

#endif // __BOARD_H
