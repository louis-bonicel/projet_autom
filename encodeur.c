
#include "stm32f4xx_conf.h"
#include "encodeur.h"


void Encodeur_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE );

	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;

	GPIO_Init( GPIOB , &GPIO_InitStructure );


	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG , ENABLE );

	SYSCFG_EXTILineConfig( EXTI_PortSourceGPIOB , EXTI_PinSource6 );

	EXTI_InitStructure.EXTI_Line	= EXTI_Line6;
	EXTI_InitStructure.EXTI_LineCmd	= ENABLE;
	EXTI_InitStructure.EXTI_Mode	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger	= EXTI_Trigger_Rising_Falling;

	EXTI_Init( &EXTI_InitStructure );

	SYSCFG_EXTILineConfig( EXTI_PortSourceGPIOB , EXTI_PinSource7 );

	EXTI_InitStructure.EXTI_Line	= EXTI_Line7;
	EXTI_InitStructure.EXTI_LineCmd	= ENABLE;
	EXTI_InitStructure.EXTI_Mode	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger	= EXTI_Trigger_Rising_Falling;

	EXTI_Init( &EXTI_InitStructure );


	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

	NVIC_Init( &NVIC_InitStructure );
}
