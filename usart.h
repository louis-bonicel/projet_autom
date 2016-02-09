/**
* @file usart.h
* @author Louis Bonicel
* @author Florian Boyer
* @author Victor Marnas
*
* @date 15 Janvier 2016
*
* @brief Declaration des fonctions pour la configuration et l'acces au port USART.
*
* @details Ce fichier regroupe les declarations des differentes fonctions
* qui seront necessaires pour communiquer en utilisant le protocole USART.
*
*/
#pragma once

#include "PrjDefinitions.h"

/** @brief OFFSET de reception du mode */
static const uint8_t MODE_OFFSET = 0;
/** @brief OFFSET de reception du signe */
static const uint8_t SIGNE_OFFSET = 0;
/** @brief OFFSET de reception du point de depart */
static const uint8_t START_OFFSET = 1;
/** @brief OFFSET de reception du point d'arrivee */
static const uint8_t END_OFFSET = 3;

/** @brief Point de depart est negatif */
static const uint8_t START_POINT_NEG = 0b0100;
/** @brief Point d'arrivee est negatif */
static const uint8_t END_POINT_NEG = 0b0001;
/** @brief Point de depart est positif */
static const uint8_t START_POINT_POS = 0b1000;
/** @brief Point d'arrivee est positif */
static const uint8_t END_POINT_POS = 0b0010;

void USART3_Config( void );
void UpdateReceivedConsigne( t_ConsigneReceived * consigne );
void SendData( t_Data data );
signed int my_printf(const char *pFormat, ...);
