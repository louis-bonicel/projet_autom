/**
* \file usart.h
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Declaration des fonctions pour la configuration et l'acces au port USART.
*
* \details Ce fichier regroupe les declarations des differentes fonctions
* qui seront necessaires pour communiquer en utilisant le protocole USART.
*
*/
#pragma once

static const uint8_t MODE_OFFSET = 0;
static const uint8_t SIGNE_OFFSET = 0;
static const uint8_t START_OFFSET = 1;
static const uint8_t END_OFFSET = 3;

static const uint8_t START_POINT_NEG = 0b0100;
static const uint8_t END_POINT_NEG = 0b0001;
static const uint8_t START_POINT_POS = 0b1000;
static const uint8_t END_POINT_POS = 0b0010;

void USART3_Config( void );
void UpdateReceivedConsigne( t_ConsigneReceived * consigne );
void SendData( t_Data data );
signed int my_printf(const char *pFormat, ...);
