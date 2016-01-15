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

/**
* \fn void ADC_Config ( void )
*
* \brief Configure les ADC pour le potentiometre et le tachymetre.
*
* \details Potentiometre	: adc_consigne	-> PB0	ADC1 channel 8
* \details Tachymetre		: adc_tachy		-> PA2	ADC1 channel 2
*/
void ADC_Config ( void )
{
	// Configuration de l'ADC.

	// Demarrage des horloges de GPIOB, GPIOA et ADC1
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 , ENABLE );

	// Structure d'initialisation des GPIOs.
	GPIO_InitTypeDef GPIO_InitStruct;

	// B0 est une entree analogique, sans Pull-Up.
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de B0
	GPIO_Init( GPIOB , &GPIO_InitStruct );

	// A2 est une entree analogique, sans Pull-Up.
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de A2
	GPIO_Init( GPIOA , &GPIO_InitStruct );

	// Nettoie la configuration existante des ADC.
	ADC_DeInit( );

	// Creation de la structure d'initialisation
	ADC_InitTypeDef ADC_InitStruct;
	// Initialisation de la dite structure.
	ADC_StructInit( &ADC_InitStruct );

	// L'ADC est en 12 bits, l'aquisition est declenchee manuellement
	// Les donnees sont alignees a droite, sans declenchement en externe.
	ADC_InitStruct.ADC_Resolution			= ADC_Resolution_12b;
	ADC_InitStruct.ADC_ContinuousConvMode	= DISABLE;
	ADC_InitStruct.ADC_DataAlign			= ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv		= DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ScanConvMode			= DISABLE;

	// Initialisation de l'ADC1 avec la structure remplie au dessus.
	ADC_Init( ADC1 , &ADC_InitStruct );

	// Demarre l'ADC.
	ADC_Cmd( ADC1 , ENABLE );

	// Reinitialise le flag de fin d'acquisition pour une future mesure.
	ADC_ClearFlag( ADC1 , ADC_FLAG_EOC );
}


/**
* \fn void ADC_GetPotValue ( uint16_t * value )
*
* \brief Cette fonction recupere une mesure d'ADC pour le potentiometre.
* \param value Pointeur vers la variable dans laquelle retourner la valeur d'ADC.
*
*/
void ADC_GetPotValue ( uint16_t * value )
{
	/*
	Puisque l'on utilise un seul ADC mais avec plusieur channels, nous devons selectionner
	lequel utiliser. Ici, le potentiometre est sur l'ADC 1 , channel 8.
	*/
	ADC_RegularChannelConfig( ADC1 , ADC_Channel_8 , 1 , ADC_SampleTime_15Cycles );

	// Demande a l'ADC de demarrer une acquisition.
	ADC_SoftwareStartConv( ADC1 );

	// Tant que la conversion n'est pas terminee (End Of Conversion == 0)
	// on attend.
	while( ADC_GetFlagStatus( ADC1 , ADC_FLAG_EOC ) == RESET );

	// La valeur lue est ecrite dans la variable value.
	*value = ADC_GetConversionValue( ADC1 );

	// Mise x 0 du flag signifiant la fin d'acquisition.
	ADC_ClearFlag( ADC1 , ADC_FLAG_EOC );
}


/**
* \fn void ADC_GetTachyValue ( uint16_t * value )
*
* \brief Recupere une mesure d'ADC pour le tachymetre et l'ecrit dans son argument.
* \param value Pointeur vers la variable dans laquelle retourner la valeur d'ADC.
*
*/
void ADC_GetTachyValue ( uint16_t * value )
{
	/*
	Puisque l'on utilise un seul ADC mais avec plusieur channels, nous devons selectionner
	lequel utiliser. Ici, le tachymetre est sur l'ADC 1 , channel 2.
	*/
	ADC_RegularChannelConfig( ADC1 , ADC_Channel_2 , 1 , ADC_SampleTime_15Cycles );

	// Demande a l'ADC de demarrer une acquisition.
	ADC_SoftwareStartConv( ADC1 );

	// Tant que la conversion n'est pas terminee (End Of Conversion == 0)
	// on attend.
	while( ADC_GetFlagStatus( ADC1 , ADC_FLAG_EOC ) == RESET );

	// La valeur lue est ecrite dans la variable value.
	*value = ADC_GetConversionValue( ADC1 );

	// Mise a 0 du flag signifiant la fin d'acquisition.
	ADC_ClearFlag( ADC1 , ADC_FLAG_EOC );
}

