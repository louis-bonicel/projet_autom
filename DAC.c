/**
* \file DAC.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Implementation des fonctions pour la configuration du DAC
*
* \details Ce fichier regroupe les implementations des differentes fonctions
* qui seront necessaires pour ecrire une valeur sur le DAC du STM.
*
*/

#include "stm32f4xx_conf.h"
#include "DAC.h"

uint16_t dac_p = 0;
uint16_t dac_n= 0;

/**
* @brief Configure les sorties DAC pour controler le moteur.
* Ce sont les ports dac_n et dac_p
* @details dac_n -> PA5	DAC2
* @details dac_p -> PA4	DAC1
*/
void DAC_Config ( void )
{
	// Structure d'initialisation des GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;
	// Structure d'initialisation du DAC.
	DAC_InitTypeDef DAC_InitStructure;

	// Demarrage de l'horloge des peripheriques GPIOA et DAC.
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_DAC , ENABLE );

	// Pin A4 et A5 comme DAC, mode analogique, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation du port avec ces parametres.
	GPIO_Init( GPIOA , &GPIO_InitStructure );

	// RAZ des parametres du DAC
	DAC_DeInit();

	// Le DAC est declenche manuellement
	DAC_InitStructure.DAC_Trigger			= DAC_Trigger_Software;
	// Avec buffer de sortie
	DAC_InitStructure.DAC_OutputBuffer		= DAC_OutputBuffer_Enable;
	// Et la generation d'onde n'est pas utilisee.
	DAC_InitStructure.DAC_WaveGeneration	= DAC_WaveGeneration_None;
	// Initalisation des channel 1 et 2 du DAC


	DAC_Init( DAC_Channel_1 , &DAC_InitStructure );
	DAC_Cmd( DAC_Channel_1 , ENABLE );

	DAC_Init( DAC_Channel_2 , &DAC_InitStructure );
	DAC_Cmd( DAC_Channel_2 , ENABLE );
}

void UpdateConsigneDAC( int16_t consigne_rpm )
{
	RPMToDAC( consigne_rpm , &dac_p , &dac_n );

	// Met à jour les channel du DAC avec la valeur de dac_p et dac_n
	DAC_SetDualChannelData( DAC_Align_12b_R , dac_n , dac_p );

	// Active le DAC pour qu'il prenne en compte la valeur.
	DAC_DualSoftwareTriggerCmd( ENABLE );
}


void RPMToDAC( int16_t consigne , uint16_t * dac_p , uint16_t * dac_n )
{
	float consigne_dac = (float) consigne * K_RPM_TO_CONSIGNE;
	if (consigne_dac > 4095.0)
		consigne_dac = 4095.0;
	if (consigne_dac < -4095.0)
			consigne_dac = -4095.0;


	*dac_p = (uint16_t)( (float)(0x0001 << (RESOLUTION - 1)) + (consigne_dac / 2.0));
	*dac_n = (uint16_t)( (float)(0x0001 << (RESOLUTION - 1)) - (consigne_dac / 2.0));
}
