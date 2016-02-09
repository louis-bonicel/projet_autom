/**
* \file main.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Programme de controle d'un banc moteur.
*
* \details Projet d'automatisme SE4, asservissement d'un moteur en vitesse.
*/

#include "PrjDefinitions.h"
#include "stm32f4xx_conf.h"
#include "usart.h"
#include "DAC.h"
#include "ADC.h"
#include "asservissement.h"
#include "board.h"
#include "encodeur.h"
#include "tachy.h"
#include "PID.h"


void Global_Config ( void );

/**
 * @var Used for interrupt flag
 */
t_PID pid;
t_Data data;

/**
* @brief Entree du programme.
* @return Rien, le int est ici pour eviter un warning GCC.
*/
int main ( void )
{
	Global_Config();

	t_ConsigneReceived consigne;
	Consigne_Init( &consigne );

	Data_Init( &data );

	PID_Init( &pid );

	int16_t sweepCounter = 0;
	uint16_t stepCounter = 0;

	while(1)
	{
		while ( flag.mainProcess )
		{
			GPIO_SetBits( GPIOE , GPIO_Pin_11 );
			UpdateValues( &data );
			int16_t valueToApply;

			switch( consigne.mode )
			{
				case NORMAL:
					data.consigneReceived = consigne.start_point;
					PID_Calculate( &pid , &data );
					UpdateConsigneDAC( &(pid.consigneOut) );
					// UpdateConsigneDAC( &(data.consigneReceived) );
					break;

				case STEP:
					if ( stepCounter < STEP_HOLD_START )
					{
						stepCounter++;
						UpdateConsigneDAC( &(consigne.start_point) );
					}
					else
					{
						if (stepCounter < STEP_HOLD_START + STEP_HOLD_STOP)
						{
							stepCounter++;
							UpdateConsigneDAC( &(consigne.end_point) );
						}
						else
							consigne.mode = NORMAL;
					}
					break;

				case SWEEP:
					if ( sweepCounter != consigne.end_point )
					{
						if (sweepCounter < consigne.end_point)
							sweepCounter++;
						else
							sweepCounter--;
					}
					else
					{
						consigne.mode = NORMAL;
					}
					UpdateConsigneDAC( &(sweepCounter) );
					break;

				case POTARD:
					valueToApply = ( data.potardValue - 2048 );
					DAC_SetChannel1Data( DAC_Align_12b_R , 2048 + valueToApply );
					DAC_SetChannel2Data( DAC_Align_12b_R , 2047 - valueToApply );
					break;
			}

			flag.mainProcess = 0;
			GPIO_ResetBits( GPIOE , GPIO_Pin_11 );
		}
		while ( flag.consigneUpdate )
		{
			UpdateReceivedConsigne( &consigne );
			if (consigne.mode != NORMAL)
			{
				sweepCounter = consigne.start_point;
				stepCounter = 0;
			}
			flag.consigneUpdate = 0;
		}
		while ( flag.sendData )
		{
			GPIO_SetBits( GPIOE , GPIO_Pin_12 );

			if (flag.UARTTXReady)
				SendData( data );
			flag.sendData = 0;

			GPIO_ResetBits( GPIOE , GPIO_Pin_12 );
		}
		while ( flag.button )
		{
			PID_Init( &pid );
			if (consigne.mode == POTARD)
				consigne.mode = NORMAL;
			else
				consigne.mode = POTARD;
			flag.button = 0;
		}
	}
}


/**
 * @brief Initialise tous les peripherique necessaires.
 */
void Global_Config ( void )
{
	// Configure l'USART
	USART3_Config();
	flag.UARTTXReady = 1;


	my_printf( "\r\n" );
	my_printf( "                                  Projet SE 4\r\n\r\n" );
	my_printf( "                                Initialisations\r\n" );
	my_printf( "UART et RX DMA Initialise avec succes !\r\n\r\n" );

	// Configure les GPIOs pour les LEDs.
	my_printf( "Initialisation LEDs et BNCs\r\n" );
	LED_Config();
	GPIO_BNC_Config();

	// Configure les GPIOs pour les LEDs.
	my_printf( "Initialisation du bouton\r\n" );
	PushButton_Config();

	// Configure les ADCs.
	my_printf( "Initialisation ADCs et Signe tachymetre\r\n" );
	ADC_Config();
	Tachy_Config();

	// Configure l'encodeur
	my_printf( "Initialisation Encodeur\r\n" );
	Encodeur_Config();

	// Configure le DAC.
	my_printf( "Initialisation DAC\r\n" );
	DAC_Config();

	// Configure les TIMERS
	my_printf( "Initialisation Timers\r\n" );
	TIM2_Init();
	TIM3_Init();
	TIM4_Init();


	my_printf( "\r\n" );
	my_printf( "                  Fin de l'initialisation des peripheriques\r\n\r\n");
}


/**
 * @brief Gere l'interrupt sur le boutton
 */
void EXTI0_IRQHandler ( void )
{
	if ( EXTI_GetITStatus( EXTI_Line0 ) != RESET )
	{
		flag.button = 1;
		EXTI_ClearITPendingBit( EXTI_Line0 );
	}
}


/**
 * @brief Handler pour buffer UART plein
 */
void DMA1_Stream1_IRQHandler ( void )
{
	if ( DMA_GetITStatus( DMA1_Stream1 , DMA_IT_TCIF1 ) )
	{
		DMA_ClearITPendingBit( DMA1_Stream1 , DMA_IT_TCIF1 );
		flag.consigneUpdate = 1;
	}
}


/**
 * @brief Handler pour buffer UART TX transmit
 */
void DMA1_Stream3_IRQHandler ( void )
{
	if ( DMA_GetITStatus( DMA1_Stream3 , DMA_IT_TCIF3 ) )
	{
		DMA_ClearITPendingBit( DMA1_Stream3 , DMA_IT_TCIF3 );
		USART_DMACmd( USART3 , USART_DMAReq_Tx , DISABLE );
		flag.UARTTXReady = 1;
	}
}

/**
 * Timer d'echantillonage @1kHz
 */
void TIM2_IRQHandler(void)
{
	GPIO_ToggleBits( GPIOE , GPIO_Pin_13 );
	if ( TIM_GetITStatus( TIM2 , TIM_IT_Update ) != RESET )
	{
		flag.mainProcess = 1;
		TIM_ClearITPendingBit( TIM2 , TIM_IT_Update );
	}
}


/**
 * Timer d'envoie des donnees @ 10Hz
 */
void TIM3_IRQHandler(void)
{
	GPIO_ToggleBits( GPIOE , GPIO_Pin_14 );
	if ( TIM_GetITStatus( TIM3 , TIM_IT_Update ) != RESET )
	{
		flag.sendData = 1;
		TIM_ClearITPendingBit( TIM3 , TIM_IT_Update );
	}
}
