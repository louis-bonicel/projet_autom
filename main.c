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

/// \var const uint8_t RESOLUTION
/// \brief Constante de résolution des DAC en nombre de bits.
const uint8_t RESOLUTION = 12;

/// \var const float K_TACHY
/// \brief Constante pour convertir une valeur d'ADC en tours par minutes.
/// Vitesse en RPM = Valeur d'ADC / K_TACHY
const float K_TACHY = 0.518;

/// \var const float K_TACHY_TO_RPM
/// \brief Constante pour convertir une valeur d'ADC en tours par minutes.
/// K_TACHY_TO_RPM = 1 / K_TACHY
const float K_TACHY_TO_RPM = 1.93003663004;

/// \var uint16_t pot_value
/// \brief Variable globale utilisée pour récupérer la valeur du potentiomètre avec STM Studio.
uint16_t pot_value = 0;

/// \var uint16_t tachy_value
/// \brief Variable globale utilisée pour récupérer la valeur du tachymètre avec STM Studio.
uint16_t tachy_value = 0;

/// \var uint16_t tachy_speed
/// \brief Variable globale utilisée pour récupérer la vitesse calculée depuis le tachymètre avec STM Studio.
uint16_t tachy_speed = 0;

/// \var uint16_t dac_p_value
/// \brief Variable globale utilisée pour récupérer la valeur de DAC sur dac_p avec STM Studio.
uint16_t dac_p_value = 0;

/// \var uint16_t dac_n_value
/// \brief Variable globale utilisée pour récupérer la valeur de DAC sur dac_n avec STM Studio.
uint16_t dac_n_value = 0;

void ADC_Config ( void );
void DAC_SetValues ( uint16_t dac_p , uint16_t dac_n );
void Tachy_to_RPM( uint16_t value_to_convert , uint16_t * speed_rpm );
void ADC_GetPotValue( uint16_t * value );
void GPIO_Config ( void );
void DAC_Config ( void );
void SysTick_Handler( void );


/**
* \fn int main ( void )
*
* \brief Entrée du programme.
*
* \return Rien, le int est ici pour éviter un warning GCC.
*/
int main ( void )
{
	// Initialise le "tick" système, qui permet de lever une interruption toutes les µs.
	SysTick_Init();
	// Configure les GPIOs pour les LEDs.
	GPIO_Config();
	// Configure les ADCs.
	ADC_Config();
	// Configure le DAC.
	DAC_Config();

    while(1)
    {
		// A chaque passage dans la boucle, inverser une LED.
    	GPIO_ToggleBits( GPIOD , GPIO_Pin_12 );

		// Récupère la valeur du potentiomètre.
    	ADC_GetPotValue( &pot_value );
		
		// Récupère la valeur du tachymètre.
    	ADC_GetTachyValue( &tachy_value );
		
		// Convertit la valeur lue du tachy en vitesse de rotation en rpm.
    	Tachy_to_RPM( tachy_value , &tachy_speed );

		// La valeur envoyée à l'ampli de commande est 1.5 * ( dac_p - dac_n ).
		// On part donc de 2048 et on ajoute / retranche une valeur définit entre 0 et 2047.
		// Ainsi, quand cette valeur est à 0, dac_p - dac_n = 0.
		// Au max, 2047, dac_p - dac_n = 4096.
    	dac_p_value = ( 0x01 << ( RESOLUTION - 1 ) ) + ( pot_value / 2 );
    	dac_n_value = ( 0x01 << ( RESOLUTION - 1 ) ) - ( pot_value / 2 );

		// Met à jour les valeurs du DAC avec les valeurs calculées précedemment.
    	DAC_SetValues( dac_p_value , dac_n_value );

		// Reboucler toutes les 1 ms.
    	delay_nms( 1 );
    }
}


/**
* \fn void ADC_GetPotValue ( uint16_t * value )
*
* \brief Cette fonction récupère une mesure d'ADC pour le potentiomètre.
* \param value Pointeur vers la variable dans laquelle retourner la valeur d'ADC.
* \todo Déplacer vers un ADC.c / ADC.h
*/
void ADC_GetPotValue ( uint16_t * value )
{
	/*
	Puisque l'on utilise un seul ADC mais avec plusieur channels, nous devons sélectionner
	lequel utiliser. Ici, le potentiomètre est sur l'ADC 1 , channel 8.
	*/
	ADC_RegularChannelConfig( ADC1 , ADC_Channel_8 , 1 , ADC_SampleTime_15Cycles );
	
	// Demande à l'ADC de démarrer une acquisition.
	ADC_SoftwareStartConv( ADC1 );

	// Tant que la conversion n'est pas terminée (End Of Conversion == 0)
	// on attend.
	while( ADC_GetFlagStatus( ADC1 , ADC_FLAG_EOC ) == RESET );

	// La valeur lue est écrite dans la variable value.
	*value = ADC_GetConversionValue( ADC1 );

	// Mise à 0 du flag signifiant la fin d'acquisition.
	ADC_ClearFlag( ADC1 , ADC_FLAG_EOC );
}


/**
* \fn void ADC_GetTachyValue ( uint16_t * value )
*
* \brief Récupère une mesure d'ADC pour le tachymètre et l'écrit dans son argument.
* \param value Pointeur vers la variable dans laquelle retourner la valeur d'ADC.
* \todo Déplacer vers un ADC.c / ADC.h
*/
void ADC_GetTachyValue ( uint16_t * value )
{
	/*
	Puisque l'on utilise un seul ADC mais avec plusieur channels, nous devons sélectionner
	lequel utiliser. Ici, le tachymètre est sur l'ADC 1 , channel 2.
	*/
	ADC_RegularChannelConfig( ADC1 , ADC_Channel_2 , 1 , ADC_SampleTime_15Cycles );
	
	// Demande à l'ADC de démarrer une acquisition.
	ADC_SoftwareStartConv( ADC1 );

	// Tant que la conversion n'est pas terminée (End Of Conversion == 0)
	// on attend.
	while( ADC_GetFlagStatus( ADC1 , ADC_FLAG_EOC ) == RESET );

	// La valeur lue est écrite dans la variable value.
	*value = ADC_GetConversionValue( ADC1 );

	// Mise à 0 du flag signifiant la fin d'acquisition.
	ADC_ClearFlag( ADC1 , ADC_FLAG_EOC );
}


/**
* \fn void Tachy_to_RPM ( uint16_t value_to_convert , uint16_t * speed_rpm )
*
* \brief Convertit une valeur de tachymètre vers une vitesse en tr/min.
* \param value_to_convert La valeur lue du tachymètre.
* \param speed_rpm Pointeur vers la variable dans laquelle retourner la vitesse.
* \todo Déplacer vers un tachy.c / tachy.h ? Pas encore sur, voir future archi.
*/
void Tachy_to_RPM ( uint16_t value_to_convert , uint16_t * speed_rpm )
{
	// La vitesse en tr/min = valeur lue par l'ADC * K_TACHY_TO_RPM (=1.93...).
	*speed_rpm = value_to_convert * K_TACHY_TO_RPM;
}


/**
* \fn void DAC_SetValues ( uint16_t dac_p , uint16_t dac_n )
*
* \brief Met à jour les valeurs du DAC contrôlant le moteur.
* \param dac_p La valeur à écrire sur le port dac_p.
* \param dac_n La valeur à écrire sur le port dac_n.
* \todo Déplacer vers un DAC.c / DAC.h
*/
void DAC_SetValues ( uint16_t dac_p , uint16_t dac_n )
{
	// Met à jour le channel 1 du DAC avec la valeur de dac_p
	DAC_SetChannel1Data( DAC_Align_12b_R , dac_p );
	// Met à jour le channel 2 du DAC avec la valeur de dac_n
	DAC_SetChannel2Data( DAC_Align_12b_R , dac_n );

	// Active le DAC pour qu'il prenne en compte la valeur.
	DAC_SoftwareTriggerCmd( DAC_Channel_1 , ENABLE );
	DAC_SoftwareTriggerCmd( DAC_Channel_2 , ENABLE );
}


/**
* \fn void GPIO_Config ( void )
*
* \brief Configure les GPIOs correspondant aux LEDs sur la carte.
* \todo Déplacer vers un GPIO.c / GPIO.h
*/
void GPIO_Config ( void )
{
	// Structure qui sera utilisée pour configurer les GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Démarrage de l'horloge du périphérique GPIO port D.
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD , ENABLE );

	// Initialise deux LEDs, présentent sur les pins PD12 et PD13, comme
	// sortie, en mode PP, @100MHz, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD , &GPIO_InitStructure );
}


/**
* \fn void ADC_Config ( void )
*
* \brief Configure les ADC pour le potentiomètre et le tachymètre.
* \todo Déplacer vers un ADC.c / ADC.h
* \details Potentiomètre	: adc_consigne	-> PB0	ADC1 channel 8
* \details Tachymètre		: adc_tachy		-> PA2	ADC1 channel 2
*/
void ADC_Config ( void )
{
	// Configuration de l'ADC.
	
	// Démarrage des horloges de GPIOB, GPIOA et ADC1
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 , ENABLE );

	// Structure d'initialisation des GPIOs.
	GPIO_InitTypeDef GPIO_InitStruct;

	// B0 est une entrée analogique, sans Pull-Up.
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de B0
	GPIO_Init( GPIOB , &GPIO_InitStruct );

	// A2 est une entrée analogique, sans Pull-Up.
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de A2
	GPIO_Init( GPIOA , &GPIO_InitStruct );

	// Nettoie la configuration existante des ADC.
	ADC_DeInit( );
	
	// Création de la structure d'initialisation
	ADC_InitTypeDef ADC_InitStruct;
	// Initialisation de la dite structure.
	ADC_StructInit( &ADC_InitStruct );

	// L'ADC est en 12 bits, l'aquisition est déclenchée manuellement
	// Les données sont alignées à droite, sans déclenchement en externe.
	ADC_InitStruct.ADC_Resolution			= ADC_Resolution_12b;
	ADC_InitStruct.ADC_ContinuousConvMode	= DISABLE;
	ADC_InitStruct.ADC_DataAlign			= ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv		= DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ScanConvMode			= DISABLE;

	// Initialisation de l'ADC1 avec la structure remplie au dessus.
	ADC_Init( ADC1 , &ADC_InitStruct );

	// Démarre l'ADC.
	ADC_Cmd( ADC1 , ENABLE );

	// Réinitialise le flag de fin d'acquisition pour une future mesure.
	ADC_ClearFlag( ADC1 , ADC_FLAG_EOC );
}


/**
* \fn void DAC_Config ( void )
*
* \brief Configure les sorties DAC pour contrôler le moteur.
* Ce sont les ports dac_n et dac_p
* \details dac_n -> PA5	DAC1
* \details dac_p -> PA4	DAC2
* \todo Déplacer vers un DAC.c / DAC.h
*/
void DAC_Config ( void )
{
	// Structure d'initialisation des GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;
	// Structure d'initialisation du DAC.
	DAC_InitTypeDef DAC_InitStructure;

	// Démarrage de l'horloge des périphériques GPIOA et DAC.
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_DAC , ENABLE );

	// Pin A4 et A5 comme DAC, mode analogique, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation du port avec ces paramètres.
	GPIO_Init( GPIOA , &GPIO_InitStructure );

	// RAZ des paramètres du DAC
	DAC_DeInit();
	
	// Le DAC est déclenché manuellement
	DAC_InitStructure.DAC_Trigger			= DAC_Trigger_Software;
	// Avec buffer de sortie
	DAC_InitStructure.DAC_OutputBuffer		= DAC_OutputBuffer_Enable;
	// Et la génération d'onde n'est pas utilisée.
	DAC_InitStructure.DAC_WaveGeneration	= DAC_WaveGeneration_None;
	// Initalisation des channel 1 et 2 du DAC
	DAC_Init( DAC_Channel_1 , &DAC_InitStructure );
	DAC_Init( DAC_Channel_2 , &DAC_InitStructure );

	// Démarrage du périphérique
	DAC_Cmd( DAC_Channel_1 , ENABLE );
	DAC_Cmd( DAC_Channel_2 , ENABLE );
}


/**
* \fn void SysTick_Handler ( void )
*
* \brief Utilisée pour gérer les delay.
* \todo Déplacer vers un misc.c / misc.h ? Pas encore sur, voir future archi.
*/
void SysTick_Handler ( void )
{
	// Called every microsecond
	TimeTick_Decrement();
}

