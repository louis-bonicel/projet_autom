/**
* \file main.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Programme de controle d'un banc moteur.
*
* \details Projet d'automatisme SE4, asservissement d'un moteur en vitesse.
*/

#include "stm32f4xx_conf.h"
#include "delay.h"
#include "usart.h"
#include "ADC.h"
#include "DAC.h"

/// \var static const float K_TACHY
/// \brief Constante pour convertir une valeur d'ADC en tours par minutes.
/// Vitesse en RPM = Valeur d'ADC / K_TACHY
static const float K_TACHY = 0.518;

/// \var static const float K_TACHY_TO_RPM
/// \brief Constante pour convertir une valeur d'ADC en tours par minutes.
/// K_TACHY_TO_RPM = 1 / K_TACHY
static const float K_TACHY_TO_RPM = 1.93003663004;

/// \var uint16_t pot_value
/// \brief Variable globale utilisee pour recuperer la valeur du potentiometre avec STM Studio.
uint16_t pot_value = 0;

/// \var uint16_t tachy_value
/// \brief Variable globale utilisee pour recuperer la valeur du tachymetre avec STM Studio.
uint16_t tachy_value = 0;

/// \var uint16_t tachy_speed
/// \brief Variable globale utilisee pour recuperer la vitesse calculee depuis le tachymetre avec STM Studio.
uint16_t tachy_speed = 0;

/// \var uint16_t dac_p_value
/// \brief Variable globale utilisee pour recuperer la valeur de DAC sur dac_p avec STM Studio.
uint16_t dac_p_value = 0;

/// \var uint16_t dac_n_value
/// \brief Variable globale utilisee pour recuperer la valeur de DAC sur dac_n avec STM Studio.
uint16_t dac_n_value = 0;

void Tachy_to_RPM( uint16_t value_to_convert , uint16_t * speed_rpm );
void GPIO_Config ( void );

void Global_Config ( void );
void SysTick_Handler( void );

/**
* \fn int main ( void )
*
* \brief Entree du programme.
*
* \return Rien, le int est ici pour eviter un warning GCC.
*/
int main ( void )
{
	Global_Config();

    while(1)
    {
		// A chaque passage dans la boucle, inverser une LED.
    	GPIO_ToggleBits( GPIOD , GPIO_Pin_12 );

		// Recupere la valeur du potentiometre.
    	ADC_GetPotValue( &pot_value );
		
		// Recupere la valeur du tachymetre.
    	ADC_GetTachyValue( &tachy_value );
		
		// Convertit la valeur lue du tachy en vitesse de rotation en rpm.
    	Tachy_to_RPM( tachy_value , &tachy_speed );

		// La valeur envoyee a l'ampli de commande est 1.5 * ( dac_p - dac_n ).
		// On part donc de 2048 et on ajoute / retranche une valeur definie entre 0 et 2047.
		// Ainsi, quand cette valeur est a 0, dac_p - dac_n = 0.
		// Au max, 2047, dac_p - dac_n = 4096.
    	dac_p_value = ( 0x01 << ( RESOLUTION - 1 ) ) + ( pot_value / 2 );
    	dac_n_value = ( 0x01 << ( RESOLUTION - 1 ) ) - ( pot_value / 2 );

		// Met Ã  jour les valeurs du DAC avec les valeurs calculees precedemment.
    	DAC_SetValues( dac_p_value , dac_n_value );

		// Reboucler toutes les 1 ms.
    	delay_nms( 1 );
    }
}


/**
 * \fn void Global_Config ( void )
 *
 * \brief Initialise tous les peripherique necessaires.
 */
void Global_Config ( void )
{
	// Initialise le "tick" systeme, qui permet de lever une interruption toutes les us.
	SysTick_Init();
	// Configure l'USART
	USART3_Config();

	my_printf( "                                  Projet SE 4                                  \n\r\n\r" );
	my_printf( "USART Initialise avec succes !\n\r" );

	// Configure les GPIOs pour les LEDs.
	my_printf( "Initialisation LEDs\n\r" );
	GPIO_Config();
	my_printf( "LEDs Initialise avec succes !\n\r" );

	// Configure les ADCs.
	my_printf( "Initialisation ADC\n\r" );
	ADC_Config();
	my_printf( "ADC Initialise avec succes !\n\r" );

	// Configure le DAC.
	my_printf( "Initialisation DAC\n\r" );
	DAC_Config();
	my_printf( "DAC Initialise avec succes !\n\r" );
}

/**
* \fn void Tachy_to_RPM ( uint16_t value_to_convert , uint16_t * speed_rpm )
*
* \brief Convertit une valeur de tachymetre vers une vitesse en tr/min.
* \param value_to_convert La valeur lue du tachymetre.
* \param speed_rpm Pointeur vers la variable dans laquelle retourner la vitesse.
* \todo Deplacer vers un tachy.c / tachy.h ? Pas encore sur, voir future archi.
*/
void Tachy_to_RPM ( uint16_t value_to_convert , uint16_t * speed_rpm )
{
	// La vitesse en tr/min = valeur lue par l'ADC * K_TACHY_TO_RPM (=1.93...).
	*speed_rpm = value_to_convert * K_TACHY_TO_RPM;
}


/**
* \fn void GPIO_Config ( void )
*
* \brief Configure les GPIOs correspondant aux LEDs sur la carte.
* \todo Deplacer vers un GPIO.c / GPIO.h
*/
void GPIO_Config ( void )
{
	// Structure qui sera utilisee pour configurer les GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Demarrage de l'horloge du peripherique GPIO port D.
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD , ENABLE );

	// Initialise deux LEDs, presentent sur les pins PD12 et PD13, comme
	// sortie, en mode PP, @100MHz, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD , &GPIO_InitStructure );
}


/**
* \fn void SysTick_Handler ( void )
*
* \brief Utilisee pour gerer les delay.
*
*/
void SysTick_Handler ( void )
{
	// Called every microsecond
	TimeTick_Decrement();
}

