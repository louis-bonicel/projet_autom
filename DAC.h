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

#ifndef __DAC_H
#define __DAC_H

/// \var static const uint8_t RESOLUTION
/// \brief Constante de resolution des DAC en nombre de bits.
static const uint8_t RESOLUTION = 12;

void DAC_Config ( void );
void Set_Consigne ( int16_t delta );
inline void DAC_SetValues ( uint16_t dac_p , uint16_t dac_n );

#endif // __DAC_H
