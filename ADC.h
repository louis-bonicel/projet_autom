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

#ifndef __ADC_H
#define __ADC_H

void ADC_Config ( volatile uint16_t * adc_buffer );

#endif // __ADC_H
