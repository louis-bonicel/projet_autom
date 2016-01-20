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

#include "stm32f4xx_conf.h"
#include "delay.h"
#include "usart.h"
#include "ADC.h"
#include "DAC.h"
#include "asservissement.h"
#include "board.h"
#include "encodeur.h"
#include "tachy.h"

typedef enum { NORMAL = (uint8_t)0b0001 , SWEEP = (uint8_t)0b0010 , STEP = (uint8_t)0b0100 } e_mode;

typedef struct Consigne{
	e_mode mode;
	int16_t start_point;
	int16_t end_point;
}Consigne;

typedef struct Data{
	int16_t speed_tachy;
	int16_t speed_encoder;
	int16_t consigne;
}Data;

volatile struct flag {
	uint8_t button			: 1;
	uint8_t consigneUpdate	: 1;
	uint8_t mainProcess		: 1;
	uint8_t sendData		: 1;
	uint16_t				: 12;
} flag;

void Global_Config ( void );
void SendData( Data * data );
void Data_Init( Data * data );
void Consigne_Init( Consigne * consigne );
void UpdateConsigne( Consigne * consigne );
void SysTick_Handler( void );
void EXTI0_IRQHandler ( void );
void DMA1_Stream1_IRQHandler ( void );
void EXTI9_5_IRQHandler ( void );
void TIM2_IRQHandler( void );

volatile uint8_t t_USART3_rx_buffer[5];
static const uint8_t MODE_OFFSET = 0;
static const uint8_t SIGNE_OFFSET = 0;
static const uint8_t START_OFFSET = 1;
static const uint8_t END_OFFSET = 4;

volatile uint16_t t_adc_buffer[2];
static const uint8_t POT_OFFSET = 0;
static const uint8_t TACHY_OFFSET = 1;

static const uint8_t START_POINT_NEG = (uint8_t)0b0100;
static const uint8_t END_POINT_NEG = (uint8_t)0b0001;
static const uint8_t START_POINT_POS = (uint8_t)0b1000;
static const uint8_t END_POINT_POS = (uint8_t)0b0010;
/**
* @brief Entree du programme.
* @return Rien, le int est ici pour eviter un warning GCC.
*/
int main ( void )
{
	Global_Config();
	
	Consigne consigne;
	Consigne_Init( &consigne );
	
	Data data;
	Data_Init( &data );

	while(1)
	{
		while ( flag.mainProcess )
		{
			flag.mainProcess = 0;
			switch( consigne -> mode )
			{
				case NORMAL:
					break;
				case STEP:
					break;
				case SWEEP:
					Sweep_Consigne( consigne->start_point , consigne->end_point);
					break;
			}			
		}
		while ( flag.consigneUpdate )
		{
			flag.consigneUpdate = 0;
			UpdateConsigne( &consigne );
		}
		while ( flag.sendData )
		{
			flag.sendData = 0;
			SendData( &data );
		}
		while ( flag.button )
		{
			flag.button = 0;
		}
	}
}


void Data_Init( Data * data )
{
	data -> speed_encoder	= 0;
	data -> speed_tachy		= 0:
	data -> consigne		= 0;
}


void Consigne_Init( Consigne * consigne )
{
	consigne -> mode		= NORMAL;
	consigne -> start_point = 0;
	consigne -> end_point	= 0;
}


void UpdateConsigne( Consigne * consigne )
{
	consigne -> mode = t_USART3_rx_buffer[MODE_OFFSET] >> 4;
	consigne -> start_point = ((uint16_t) t_USART3_rx_buffer[START_OFFSET] << 8 ) | ((uint16_t) t_USART3_rx_buffer[START_OFFSET + 1]);
	consigne -> start_point = ((uint16_t) t_USART3_rx_buffer[END_OFFSET] << 8 ) | ((uint16_t) t_USART3_rx_buffer[END_OFFSET + 1]);
	
	if (t_USART3_rx_buffer[SIGNE_OFFSET] & START_POINT_NEG)
	{
		consigne->start_point = -( consigne->start_point );
	}
	if (t_USART3_rx_buffer[SIGNE_OFFSET] & END_POINT_NEG)
	{
		consigne->end_point = -( consigne->end_point );
	}
}


void SendData( Data * data )
{
	uint8_t to_send[5];
	
	uint8_t signe = 0;
	uint16_t speed_tachy_to_send = 0;
	uint16_t speed_encoder_to_send = (uint16_t)(data -> speed_encoder);
	
	if (data -> speed_tachy) < 0
	{
		signe |= START_POINT_NEG;
		speed_tachy_to_send = (uint16_t)(-(data -> speed_tachy));
	}
	else
	{
		signe |= START_POINT_POS;
		speed_tachy_to_send = (uint16_t)(data -> speed_tachy);
	}
	
	if (data -> speed_encoder) < 0
	{
		signe |= END_POINT_NEG;
		speed_encoder_to_send = (uint16_t)(-(data -> speed_encoder));
	}
	else
	{
		signe |= END_POINT_POS;
		speed_encoder_to_send = (uint16_t)(data -> speed_encoder);
	}
	
	to_send[ SIGNE_OFFSET ] = signe;
	to_send[ START_OFFSET ] = (uint8_t)(speed_tachy_to_send >> 8);
	to_send[ START_OFFSET + 1 ] = (uint8_t)(speed_tachy_to_send & 0x00FF);
	to_send[ END_OFFSET ] = (uint8_t)(speed_encoder_to_send >> 8);
	to_send[ END_OFFSET + 1 ] = (uint8_t)(speed_encoder_to_send & 0x00FF);
	
	my_printf( to_send );
}


void TIMs_Init( void )
{
	NVIC_InitTypeDef 			NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;

	// Enable the TIM2&3 global Interrupt

	NVIC_InitStructure.NVIC_IRQChannel						= TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel						= TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_Init(&NVIC_InitStructure);

	// TIM2 clock enable

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// Time base configuration

	TIM_TimeBaseStructure.TIM_Period		= 1000 - 1; 	// TS in µs (1 milli)
	TIM_TimeBaseStructure.TIM_Prescaler		= 168 - 1; 		// 168 MHz Clock down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit( TIM2 , &TIM_TimeBaseStructure );

	// TIM IT enable

	TIM_ITConfig( TIM2 , TIM_IT_Update , ENABLE );

	// TIM2 enable counter

	TIM_Cmd( TIM2 , ENABLE );



	// TIM3 clock enable

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);

	// Time base configuration

	TIM_TimeBaseStructure.TIM_Period		= 10000 - 1; 	// TS in µs (10 milli)
	TIM_TimeBaseStructure.TIM_Prescaler		= 168 - 1; 		// 168 MHz Clock down to 1 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit( TIM3 , &TIM_TimeBaseStructure );

	// TIM IT enable

	TIM_ITConfig( TIM3 , TIM_IT_Update , ENABLE );

	// TIM2 enable counter

	TIM_Cmd( TIM3 , ENABLE );
}


/**
 * @brief Initialise tous les peripherique necessaires.
 */
void Global_Config ( void )
{
	// Initialise le "tick" systeme, qui permet de lever une interruption toutes les us.
	SysTick_Init();
	// Configure l'USART
	USART3_Config( t_USART3_rx_buffer );

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
	ADC_Config( t_adc_buffer );
	Tachy_Config();

	// Configure l'encodeur
	my_printf( "Initialisation Encodeur\r\n" );
	Encodeur_Config();

	// Configure le DAC.
	my_printf( "Initialisation DAC\r\n" );
	DAC_Config();

	my_printf( "\r\n" );
	my_printf( "                  Fin de l'initialisation des peripheriques\r\n\r\n");
}


void Sweep_Consigne ( int16_t min , int16_t max )
{
	int16_t consigne = ( min < max ) ? min : max;
	int16_t tachy_value = 0;
	int16_t rpm_speed = 0;

	my_printf( "                              Demarrage du sweep\r\n\r\n" );
	my_printf( "Consigne,ADC_tachy,RPM\r\n" );

	Set_Consigne( min );
	delay_nms( 100 );

	int16_t fin = min < max ? max : min;

	while ( !flag.button && consigne <= fin )
	{
		Set_Consigne( consigne );
		delay_nms( 1 );
		GetTachyValue( &tachy_value );

		Tachy_to_RPM( tachy_value , &rpm_speed );

		my_printf( "%i,%i,%i\r\n" , consigne , tachy_value , rpm_speed );

		consigne++;
	}
	flag.button = 0;
}


void GetTachyValue ( int16_t * tachy_value )
{
	uint16_t adc_value = t_adc_buffer[0];

	uint8_t signe = GPIO_ReadInputDataBit( GPIOA , GPIO_Pin_1 );

	*tachy_value = signe == Bit_RESET ? -adc_value : adc_value;
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
		EXTI_ClearITPendingBit( EXTI_Line0 );
		flag.button = 0b1;
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
		flag.consigneUpdate;
		DMA_ClearITPendingBit( DMA1_Stream1 , DMA_IT_TCIF1 );
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
