/**
* \file asservissement.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Implementation des fonctions pour l'asservissement
*
* \details
*
*/

#include "stm32f4xx_conf.h"
#include "PrjDefinitions.h"
#include "asservissement.h"
#include "delay.h"
#include "DAC.h"
#include "ADC.h"
#include "usart.h"
#include "tachy.h"
#include "board.h"



void UpdateValues( t_Data * data )
{
	data -> speed_encoder	= 0;
	Tachy_to_RPM( t_adc_buffer[TACHY_OFFSET] , &(data->speed_tachy));

	// DAC_SetDualChannelData( DAC_Align_12b_R , t_DAC_BUFFER[1] , t_DAC_BUFFER[0] );

	DAC_DualSoftwareTriggerCmd( ENABLE );

	if (GPIO_ReadInputDataBit( GPIOA , GPIO_Pin_1 ) == Bit_RESET)
	{
		data -> speed_tachy = -(data -> speed_tachy);
	}

	/**
	 * @todo Add rotary encoder value, and determine best value to
	 * return in speed.
	 */
	data -> speed = data -> speed_tachy;
}


void TIM2_Init( void )
{
	NVIC_InitTypeDef 			NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2 , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3 , ENABLE );

	// Enable the TIM2&3 global Interrupt
	NVIC_InitStructure.NVIC_IRQChannel						= TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period		= 1000 - 1; 	// TS in us (1 milli)
	TIM_TimeBaseStructure.TIM_Prescaler		= 84 - 1; 		// 84 MHz Clock down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit( TIM2 , &TIM_TimeBaseStructure );

	TIM_ITConfig( TIM2 , TIM_IT_Update , ENABLE );

	TIM_Cmd( TIM2 , ENABLE );
}


void TIM3_Init( void )
{
	NVIC_InitTypeDef 			NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3 , ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel						= TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period		= 100000 - 1; 	// TS in us (100 millis)
	TIM_TimeBaseStructure.TIM_Prescaler		= 84 - 1; 		// 84 MHz Clock down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit( TIM3 , &TIM_TimeBaseStructure );

	TIM_ITConfig( TIM3 , TIM_IT_Update , ENABLE );

	TIM_Cmd( TIM3 , ENABLE );
}
