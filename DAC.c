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

/**
* \fn void DAC_Config ( void )
*
* \brief Configure les sorties DAC pour controler le moteur.
* Ce sont les ports dac_n et dac_p
* \details dac_n -> PA5	DAC1
* \details dac_p -> PA4	DAC2
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
	DAC_Init( DAC_Channel_2 , &DAC_InitStructure );

	// Demarrage du peripherique
	DAC_Cmd( DAC_Channel_1 , ENABLE );
	DAC_Cmd( DAC_Channel_2 , ENABLE );
}


/**
* \fn void DAC_SetValues ( uint16_t dac_p , uint16_t dac_n )
*
* \brief Met a jour les valeurs du DAC controlant le moteur.
* \param dac_p La valeur a ecrire sur le port dac_p.
* \param dac_n La valeur a ecrire sur le port dac_n.
*/
void DAC_SetValues ( uint16_t dac_p , uint16_t dac_n )
{
	// Met a jour le channel 1 du DAC avec la valeur de dac_p
	DAC_SetChannel1Data( DAC_Align_12b_R , dac_p );
	// Met a jour le channel 2 du DAC avec la valeur de dac_n
	DAC_SetChannel2Data( DAC_Align_12b_R , dac_n );

	// Active le DAC pour qu'il prenne en compte la valeur.
	DAC_SoftwareTriggerCmd( DAC_Channel_1 , ENABLE );
	DAC_SoftwareTriggerCmd( DAC_Channel_2 , ENABLE );
}
