
#include "stm32f4xx_conf.h"
#include "encodeur.h"


void Encodeur_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE );

	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

	GPIO_Init( GPIOB , &GPIO_InitStructure );
}

void TIM4_Init ( void )
{
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4 , ENABLE );

	TIM_TimeBaseStructure.TIM_Period = 0xffff;
	TIM_TimeBaseStructure.TIM_Prescaler = 0; // Don't care
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // Neither
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // CountUp, but doesn't really matter
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_EncoderInterfaceConfig( TIM4 , TIM_EncoderMode_TI12 , TIM_ICPolarity_Rising , TIM_ICPolarity_Rising );

	TIM_Cmd( TIM4 , ENABLE );

	TIM4 -> CNT = 0;
}
