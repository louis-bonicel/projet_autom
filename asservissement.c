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
#include "encodeur.h"
#include "DAC.h"
#include "ADC.h"
#include "usart.h"
#include "tachy.h"
#include "board.h"


/**
 * @brief Mise a jour des valeurs de vitesse avec les dernieres dispo de l'ADC
 * et de l'encodeur.
 * @param data Structure qui sera remplie.
 */
void UpdateValues( t_Data * data )
{
	data -> speed_encoder = (int16_t)(( (float)(TIM4 -> CNT) * 60000.0 ) / K_TICK_PER_TURN);
	// data -> speed_encoder = (int16_t)TIM4 -> CNT;
	TIM4 -> CNT = 0;

	data->speed_tachy = t_adc_buffer[TACHY_OFFSET];
	Tachy_to_RPM( t_adc_buffer[TACHY_OFFSET] , &(data->speed_tachy));

	if (GPIO_ReadInputDataBit( GPIOA , GPIO_Pin_1 ) == Bit_RESET)
	{
		data -> speed_tachy = -(data -> speed_tachy);
	}

	data -> potardValue = t_adc_buffer[POT_OFFSET];
	/**
	 * @todo Determine best value to return in speed.
	 */
	if ( ( data -> speed_tachy > 150 ) || ( data -> speed_tachy < -150 ) )
		data -> speed = ( data -> speed_encoder + data -> speed_tachy ) / 2;
	else
		data -> speed = data -> speed_encoder;
}


void Correcteur( int16_t * consigne_output , t_Data * data )
{
	static int16_t error_precedente = 0;
	static int16_t consigne_output_precedente = 0;

	int16_t error = data->consigneReceived - data->speed;
	*consigne_output =  1.7098826 * error - 1.675685 * error_precedente + consigne_output_precedente;

	if ( *consigne_output > 25000 )
		*consigne_output = 25000;
	if ( *consigne_output < -25000 )
		*consigne_output = -25000;

	error_precedente = error;
	consigne_output_precedente = *consigne_output;
}

/**
 * @brief Initialise le TIMER 2 aux parametres definis.
 * @details 1kHz avec interruption.
 */
void TIM2_Init( void )
{
	NVIC_InitTypeDef 			NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG , ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2 , ENABLE );

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


/**
 * @brief Initialise le TIMER 3 aux parametres definis.
 * @details 10Hz avec interruption.
 */
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

	TIM_TimeBaseStructure.TIM_Period		= 50000 - 1; 	// TS in us (100 millis)
	TIM_TimeBaseStructure.TIM_Prescaler		= 168 - 1; 		// 84 MHz Clock down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit( TIM3 , &TIM_TimeBaseStructure );

	TIM_ITConfig( TIM3 , TIM_IT_Update , ENABLE );

	TIM_Cmd( TIM3 , ENABLE );
}
