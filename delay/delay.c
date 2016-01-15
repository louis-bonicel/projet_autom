#include "delay.h"

static __IO uint32_t sysTickCounter;

void SysTick_Init(void)
{
	while ( SysTick_Config( SystemCoreClock / 1000000 ) != 0 )
	{
	}
	// One SysTick interrupt now equals 1us (168 ticks)
}

/*
 * This method needs to be called in the SysTick_Handler
 */
void TimeTick_Decrement( void )
{
	if ( sysTickCounter ) // While sysTickCounter != 0
	{
		sysTickCounter--; // Decrement it
	}
}

void delay_nus( uint32_t n )
{
	// Set sysTickCounter to n us
	sysTickCounter = n;

	while ( sysTickCounter )
	{
	}
}

void delay_1ms( void )
{
	// Set sysTickCounter to 1000 us = 1 ms
	sysTickCounter = 1000;

	while ( sysTickCounter )
	{
	}
}

void delay_nms( uint32_t n )
{
	while (n--)
	{
		delay_1ms();
	}
}
