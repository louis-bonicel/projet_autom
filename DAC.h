/**
* \file DAC.h
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Declaration des fonctions pour la configuration du DAC
*
* \details Ce fichier regroupe les declarations des differentes fonctions
* qui seront necessaires pour ecrire une valeur sur le DAC du STM.
*
*/

#pragma once

/// @todo a determiner sur maquette
static const float K_RPM_TO_CONSIGNE = 0.4489154;

/// \var static const uint8_t RESOLUTION
/// \brief Constante de resolution des DAC en nombre de bits.
static const uint8_t RESOLUTION = 12;

void DAC_Config ( void );
void UpdateConsigneDAC( int16_t consigne_rpm );
void RPMToDAC( int16_t consigne, uint16_t * dac_p , uint16_t * dac_n );
