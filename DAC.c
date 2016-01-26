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
#include "usart.h"
#include "DAC.h"

volatile uint16_t dac_p = 0;
volatile uint16_t dac_n = 0;

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

	// DMA_InitTypeDef DMA_InitStructure;

	// Demarrage de l'horloge des peripheriques GPIOA et DAC.
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_DAC , ENABLE );
	// RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1 , ENABLE );

	// Pin A4 et A5 comme DAC, mode analogique, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation du port avec ces parametres.
	GPIO_Init( GPIOA , &GPIO_InitStructure );

	// RAZ des parametres du DAC
	DAC_DeInit();

	// Le DAC est declenche manuellement
	DAC_InitStructure.DAC_Trigger			= DAC_Trigger_None;
	// Avec buffer de sortie
	DAC_InitStructure.DAC_OutputBuffer		= DAC_OutputBuffer_Enable;
	// Et la generation d'onde n'est pas utilisee.
	DAC_InitStructure.DAC_WaveGeneration	= DAC_WaveGeneration_None;
	// Initalisation des channel 1 et 2 du DAC
/*
	DMA_DeInit( DMA1_Stream5 );
	DMA_InitStructure.DMA_Channel				= DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr 	= (uint32_t)0x40007408;
	DMA_InitStructure.DMA_Memory0BaseAddr	 	= (uint32_t)&dac_p;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize			= 1;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_High;
	DMA_Init( DMA1_Stream5 , &DMA_InitStructure );

	DMA_Cmd( DMA1_Stream5 , ENABLE );*/

	DAC_Init( DAC_Channel_1 , &DAC_InitStructure );
	DAC_Cmd( DAC_Channel_1 , ENABLE );

	// DAC_DMACmd( DAC_Channel_1 , ENABLE );
/*
	DMA_DeInit( DMA1_Stream6 );
	DMA_InitStructure.DMA_Channel				= DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr 	= (uint32_t)0x40007414;
	DMA_InitStructure.DMA_Memory0BaseAddr	 	= (uint32_t)&dac_n;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize			= 1;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_High;
	DMA_Init( DMA1_Stream6 , &DMA_InitStructure );

	DMA_Cmd( DMA1_Stream6 , ENABLE );*/

	DAC_Init( DAC_Channel_2 , &DAC_InitStructure );
	DAC_Cmd( DAC_Channel_2 , ENABLE );

	// DAC_DMACmd( DAC_Channel_2 , ENABLE );*/
}

void UpdateConsigneDAC( int16_t consigne_rpm )
{
	RPMToDAC( consigne_rpm );

	// Met à jour les channel du DAC avec la valeur de dac_p et dac_n
	DAC_SetChannel1Data( DAC_Align_12b_R , dac_p );
	DAC_SetChannel2Data( DAC_Align_12b_R , dac_n );
}


void RPMToDAC( int16_t consigne )
{
	float consigne_dac = (float) consigne * K_RPM_TO_CONSIGNE;
	if (consigne_dac > 4095.0)
		consigne_dac = 4095.0;
	if (consigne_dac < -4095.0)
			consigne_dac = -4095.0;


	dac_p = (uint16_t)( 2048.0 + (consigne_dac / 2.0));
	dac_n = (uint16_t)( 2048.0 - (consigne_dac / 2.0));
}
