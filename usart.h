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

#ifndef __USART_H
#define __USART_H

void USART3_Config( void );
signed int my_printf(const char *pFormat, ...);

#endif // __USART_H
