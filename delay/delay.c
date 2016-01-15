/**
 * \file delay.c
 *
 * \author Inconnu
 *
 * \brief Implementation d'un delay par le SysTick.
 *
 */

#include "delay.h"

/// \var static __IO uint32_t sysTickCounter
/// \brief Variable globale qui contient le nombre de tick systeme.
static __IO uint32_t sysTickCounter;


/**
 * \fn void SysTick_Init ( void )
 *
 * \brief Initialisation du Tick a 1us
 */
void SysTick_Init ( void )
{
	while ( SysTick_Config( SystemCoreClock / 1000000 ) != 0 )
	{
	}
	// One SysTick interrupt now equals 1us (168 ticks)
}


/**
 * \fn void TimeTick_Decrement ( void )
 *
 * \brief Cette fonction decompte le compteur de tick a chaque appel.
 *
 * \warning This method needs to be called in the SysTick_Handler
 */
void TimeTick_Decrement ( void )
{
	if ( sysTickCounter ) // While sysTickCounter != 0
	{
		sysTickCounter--; // Decrement it
	}
}


/**
 * \fn void delay_nus ( uint32_t n )
 *
 * \brief Delay de n us.
 *
 * \details Met la variale globale SysTickCounter a la valeur n. La fonction
 * TimeTick_Decrement decremente cette valeur toutes les us, ainsi
 * un delay de n us est parametre.
 *
 * \param n Un entier non signe sur 32bits, valeur du delai en microsecondes.
 */
void delay_nus ( uint32_t n )
{
	// Set sysTickCounter to n us
	sysTickCounter = n;

	while ( sysTickCounter )
	{
	}
}


/**
 * \fn void delay_1ms ( void )
 *
 * \brief Delay de 1 ms.
 *
 * \details Met la variale globale SysTickCounter a la valeur 1000. La fonction
 * TimeTick_Decrement decremente cette valeur toutes les us, ainsi
 * un delay de 1 milliseconde est parametre.
 *
 */
void delay_1ms ( void )
{
	// Set sysTickCounter to 1000 us = 1 ms
	sysTickCounter = 1000;

	while ( sysTickCounter )
	{
	}
}


/**
 * \fn void delay_nms ( uint32_t n )
 *
 * \brief Delay de n ms.
 *
 * \details Appelle n delai de 1 milliseconde.
 *
 * \param n Le temps en milliseconde du delai.
 */
void delay_nms ( uint32_t n )
{
	while (n--)
	{
		delay_1ms();
	}
}
