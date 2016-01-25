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
#include "delay.h"
#include "usart.h"
#include "DAC.h"
#include "ADC.h"
#include "asservissement.h"
#include "board.h"
#include "encodeur.h"
#include "tachy.h"
#include "PID.h"

void Global_Config ( void );

volatile t_Flag flag;
t_PID pid;
/**
* @brief Entree du programme.
* @return Rien, le int est ici pour eviter un warning GCC.
*/
int main ( void )
{
	Global_Config();

	t_ConsigneReceived consigne;
	Consigne_Init( &consigne );

	t_Data data;
	Data_Init( &data );


	PID_Init( &pid );

	int16_t sweepCounter = 0;
	uint16_t stepCounter = 0;

	while(1)
	{
		while ( flag.mainProcess )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_12 );
			UpdateValues( &data );

			switch( consigne.mode )
			{
				case NORMAL:
					data.consigneReceived = consigne.start_point;
					PID_Calculate( &pid , data );
					UpdateConsigneDAC( pid.consigneOut );
					break;

				case STEP:
					if ( stepCounter < STEP_HOLD_START )
					{
						stepCounter++;
						UpdateConsigneDAC( consigne.start_point );
					}
					else
					{
						if (stepCounter < STEP_HOLD_START + STEP_HOLD_STOP)
						{
							stepCounter++;
							UpdateConsigneDAC( consigne.end_point );
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
					UpdateConsigneDAC( sweepCounter );
					break;
			}

			flag.mainProcess = 0;
		}
		while ( flag.consigneUpdate )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_14 );
			UpdateReceivedConsigne( &consigne );

			sweepCounter = consigne.start_point;
			stepCounter = 0;

			flag.consigneUpdate = 0;
		}
		while ( flag.sendData )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_13 );
			SendData( data );
			flag.sendData = 0;
		}
		while ( flag.button )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_15 );
			PID_Init( &pid );
			flag.button = 0;
		}
	}
}


/**
 * @brief Initialise tous les peripherique necessaires.
 */
void Global_Config ( void )
{
	// Initialise le "tick" systeme, qui permet de lever une interruption toutes les us.
	SysTick_Init();
	// Configure l'USART
	USART3_Config();


	my_printf( "\r\n" );
	my_printf( "                                  Projet SE 4\r\n\r\n" );
	my_printf( "                                Initialisations\r\n" );
	my_printf( "UART et RX DMA Initialise avec succes !\r\n\r\n" );

	// Configure les GPIOs pour les LEDs.
	my_printf( "Initialisation LEDs\r\n" );
	LED_Config();

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


	my_printf( "\r\n" );
	my_printf( "                  Fin de l'initialisation des peripheriques\r\n\r\n");

	// Configure les TIMERS
	my_printf( "Initialisation Timers\r\n" );
	TIM2_Init();
	TIM3_Init();
	TIM4_Init();
}


void Sweep_Consigne ( int16_t min , int16_t max )
{
	int16_t consigne = ( min < max ) ? min : max;
	int16_t tachy_value = 0;
	int16_t rpm_speed = 0;

	my_printf( "                              Demarrage du sweep\r\n\r\n" );
	my_printf( "Consigne,ADC_tachy,RPM\r\n" );

	/// @todo replace
	// Set_Consigne( min );
	delay_nms( 100 );

	int16_t fin = min < max ? max : min;

	while ( !flag.consigneUpdate && consigne <= fin )
	{
		/// @todo replace
		// Set_Consigne( consigne );
		delay_nms( 1 );
		/// @todo replace
		// GetTachyValue( &tachy_value );

		Tachy_to_RPM( tachy_value , &rpm_speed );

		my_printf( "%i,%i,%i" , consigne , tachy_value , rpm_speed );

		consigne++;
	}
}

/**
* @brief Utilisee pour gerer les delay.
*/
void SysTick_Handler ( void )
{
	// Called every microsecond
	TimeTick_Decrement();
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
 * @brief Handler sur l'interrupt venant de l'encodeur
 * 6 = PB6 => Enc_A
 * 7 = PB7 => Enc_B
 */
void EXTI9_5_IRQHandler ( void )
{
	// Si changement d'etat sur enc_a ( bas -> haut ou haut -> bas )
	if ( EXTI_GetITStatus( EXTI_Line6 ) != RESET )
	{
		EXTI_ClearITPendingBit( EXTI_Line6 );

	}
	// Si changement d'etat sur enc_b ( bas -> haut ou haut -> bas )
	if ( EXTI_GetITStatus( EXTI_Line7 ) != RESET )
	{
		EXTI_ClearITPendingBit( EXTI_Line7 );
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


void TIM2_IRQHandler(void)
{
	if ( TIM_GetITStatus( TIM2 , TIM_IT_Update ) != RESET )
	{
		flag.mainProcess = 1;
		TIM_ClearITPendingBit( TIM2 , TIM_IT_Update );
	}
}


void TIM3_IRQHandler(void)
{
	if ( TIM_GetITStatus( TIM3 , TIM_IT_Update ) != RESET )
	{
		flag.sendData = 1;
		TIM_ClearITPendingBit( TIM3 , TIM_IT_Update );
	}
}
