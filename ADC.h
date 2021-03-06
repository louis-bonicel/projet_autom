/**
* \file ADC.h
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Declaration des fonctions pour la configuration de l'ADC
*
* \details Ce fichier regroupe les declarations des differentes fonctions
* qui seront necessaires pour lire une valeur depuis les ADC du STM.
*
*/

#pragma once

static const uint8_t TACHY_OFFSET = 0;
static const uint8_t POT_OFFSET = 1;
static const uint8_t AWD_DELTA = 10;
volatile uint16_t t_adc_buffer[2];

void ADC_Config ( void );
