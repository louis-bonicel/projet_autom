/**
* \file asservissement.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Implementation des fonctions pour l'asservissement
*
* \details
*
*/

#include "stm32f4xx_conf.h"
#include "asservissement.h"
#include "delay.h"
#include "DAC.h"
#include "ADC.h"
#include "usart.h"
#include "tachy.h"

void Sweep_Consigne ( int16_t consigne_min , int16_t consigne_max )
{
	int16_t consigne = 0;
	int16_t tachy_value = 0;
	int16_t rpm_speed = 0;

	my_printf( "                                Starting Sweep                                 \r\n\r\n" );
	my_printf( "Consigne,ADC_tachy,RPM\r\n" );

	Set_Consigne( consigne_min );
	delay_nms( 100 );

	for ( consigne = consigne_min ; consigne <= consigne_max ; consigne++ )
	{
		Set_Consigne( consigne );
		delay_nms( 1 );
		GetTachyValue( &tachy_value );
		Tachy_to_RPM( tachy_value , &rpm_speed );

		my_printf( "%i,%i,%i\r\n" , consigne , tachy_value , rpm_speed );
	}
}
