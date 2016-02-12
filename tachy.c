
#include "stm32f4xx_conf.h"
#include "tachy.h"
#include "ADC.h"


/**
 * @brief Configure l'entree signe du Tachymetre.
 */
void Tachy_Config( void )
{
	// Signe sur PA1
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;

	GPIO_Init( GPIOA , &GPIO_InitStructure );
}

/**
* @deprecated N'est peut etre pas bonne.
* @brief Convertit une valeur de tachymetre vers une vitesse en tr/min.
* @param value_to_convert La valeur lue du tachymetre.
* @param speed_rpm Pointeur vers la variable dans laquelle retourner la vitesse.
*/
void Tachy_to_RPM_old ( int16_t value_to_convert , volatile int16_t * speed_rpm )
{
	// La vitesse en tr/min = valeur lue par l'ADC * K_TACHY_TO_RPM (=1.93...).
	*speed_rpm = (int16_t) ( (float) value_to_convert * K_TACHY_TO_RPM );
}

/**
* @brief Convertit une valeur de tachymetre vers une vitesse en tr/min.
* @param value_to_convert La valeur lue du tachymetre.
* @param speed_rpm Pointeur vers la variable dans laquelle retourner la vitesse.
*/
void Tachy_to_RPM ( int16_t value_to_convert , volatile int16_t * speed_rpm )
{
	// La vitesse en tr/min = valeur lue par l'ADC * K_TACHY_TO_RPM (=1.93...).
	/// *speed_rpm = (int16_t) ( value_to_convert * 1.72 );
	*speed_rpm = (int16_t) ( (float) value_to_convert * K_TACHY_TO_RPM );
}
