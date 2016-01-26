/**
* \file ADC.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Implementation des fonctions pour la configuration de l'ADC
*
* \details Ce fichier regroupe l'implementation des differentes fonctions
* qui seront necessaires pour lire une valeur depuis les ADC du STM.
*
*/

#include "stm32f4xx_conf.h"
#include "ADC.h"


/**
* @brief Configure les ADC pour le potentiometre et le tachymetre.
*
* @details Potentiometre	: adc_consigne	-> PB0	ADC1 channel 8
* @details Tachymetre		: adc_tachy		-> PA2	ADC1 channel 2
* @param adc_buffer Buffer pour la reception DMA
*/
void ADC_Config ( void )
{
	// Configuration de l'ADC.

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStruct;
	GPIO_InitTypeDef GPIOA_InitStruct;
	GPIO_InitTypeDef GPIOB_InitStruct;
	DMA_InitTypeDef DMA_InitStructure;

	// Demarrage des horloges de GPIOB, GPIOA et ADC1
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 , ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 , ENABLE);


	// B0 est une entree analogique, sans Pull-Up.
	GPIOB_InitStruct.GPIO_Pin	= GPIO_Pin_0;
	GPIOB_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIOB_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de B0
	GPIO_Init( GPIOB , &GPIOB_InitStruct );

	// A2 est une entree analogique, sans Pull-Up.
	GPIOA_InitStruct.GPIO_Pin	= GPIO_Pin_2;
	GPIOA_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIOA_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de A2
	GPIO_Init( GPIOA , &GPIOA_InitStruct );

	DMA_DeInit( DMA2_Stream0 );
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0x4001204C;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&t_adc_buffer[0];
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);

	DMA_Cmd( DMA2_Stream0 , ENABLE );


	// Nettoie la configuration existante des ADC.
	ADC_DeInit( );

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// L'ADC est en 12 bits, l'aquisition est declenchee manuellement
	// Les donnees sont alignees a droite, sans declenchement en externe.
	ADC_InitStruct.ADC_Resolution			= ADC_Resolution_12b;
	ADC_InitStruct.ADC_DataAlign			= ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ScanConvMode			= ENABLE;
	ADC_InitStruct.ADC_ContinuousConvMode	= ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv		= ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_NbrOfConversion		= 2;

	// Initialisation de l'ADC1 avec la structure remplie au dessus.
	ADC_Init( ADC1 , &ADC_InitStruct );

	ADC_DMACmd( ADC1 , ENABLE );

	ADC_RegularChannelConfig( ADC1 , ADC_Channel_2 ,  1 , ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig( ADC1 , ADC_Channel_8 ,  2 , ADC_SampleTime_480Cycles );

	ADC_DMARequestAfterLastTransferCmd( ADC1 , ENABLE );


	// Demarre l'ADC.
	ADC_Cmd( ADC1 , ENABLE );

	ADC_SoftwareStartConv(ADC1);
}

