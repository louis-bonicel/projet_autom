
#include "stm32f4xx_conf.h"
#include "tachy.h"
#include "ADC.h"


void Tachy_Config( void )
{
	ADC_Config();

	// Signe sur PA1
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;

	GPIO_Init( GPIOA , &GPIO_InitStructure );
}


void GetTachyValue ( int16_t * tachy_value )
{
	uint16_t adc_value = 0;
	ADC_GetValue( ADC_Channel_2 , &adc_value );

	uint8_t signe = GPIO_ReadInputDataBit( GPIOA , GPIO_Pin_1 );

	*tachy_value = signe == Bit_RESET ? -adc_value : adc_value;
}


/**
* \fn void Tachy_to_RPM ( int16_t value_to_convert , int16_t * speed_rpm )
*
* \brief Convertit une valeur de tachymetre vers une vitesse en tr/min.
* \param value_to_convert La valeur lue du tachymetre.
* \param speed_rpm Pointeur vers la variable dans laquelle retourner la vitesse.
* \todo Deplacer vers un tachy.c / tachy.h ? Pas encore sur, voir future archi.
*/
void Tachy_to_RPM ( int16_t value_to_convert , int16_t * speed_rpm )
{
	// La vitesse en tr/min = valeur lue par l'ADC * K_TACHY_TO_RPM (=1.93...).
	*speed_rpm = (int16_t) ( (float) value_to_convert * K_TACHY_TO_RPM );
}
