
#include "stm32f4xx_conf.h"
#include "board.h"
#include "ADC.h"


/**
* @brief Configure les GPIOs correspondant aux LEDs sur la carte.
*/
void LED_Config ( void )
{
	// Structure qui sera utilisee pour configurer les GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Demarrage de l'horloge du peripherique GPIO port D.
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOD , ENABLE );

	// Initialise les 4 LEDs, presentent sur les pins PD13,12,14,15, comme
	// sortie, en mode PP, @100MHz, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOD , &GPIO_InitStructure );
}


/**
 * @brief Initialise le boutton poussoir de la board sur PA0
 */
void PushButton_Config ( void )
{
	// Structure qui sera utilisee pour configurer les GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Structure pour l'initialisation de l'interrupt du boutton
	EXTI_InitTypeDef EXTI_InitStructure;

	// Structure pour l'initialisation de l'interrupt du boutton
	NVIC_InitTypeDef NVIC_InitStructure;

	// Demarrage de l'horloge du peripherique GPIO port A
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA , ENABLE );

	// Initialise le boutton, present sur le pin PA0, comme
	// entrée, en mode PP, @100MHz, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOA , &GPIO_InitStructure );

	// Demarre l'horologe du peripherique SysConfid, gerant les interrupt
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG , ENABLE );

	// Connecte EXTI Line 0 sur PA0
	SYSCFG_EXTILineConfig( EXTI_PortSourceGPIOA , EXTI_PinSource0 );

	// Parametre EXTI Line 0 comme interrupt sur front descendant ?
	/// @todo Comprendre...
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init( &EXTI_InitStructure );

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/// @todo Revoir les prios
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;

	NVIC_Init( &NVIC_InitStructure );
}


void GPIO_BNC_Config ( void )
{
	// Structure qui sera utilisee pour configurer les GPIOs.
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Demarrage de l'horloge du peripherique GPIO port D.
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOE , ENABLE );

	// Initialise les 4 LEDs, presentent sur les pins PD13,12,14,15, comme
	// sortie, en mode PP, @100MHz, sans Pull-Up.
	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode =	GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd =	GPIO_PuPd_NOPULL;
	GPIO_Init( GPIOE , &GPIO_InitStructure );
}
