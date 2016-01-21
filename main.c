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
#include "DAC.h"
#include "asservissement.h"
#include "board.h"
#include "encodeur.h"
#include "tachy.h"

typedef enum { NORMAL = (uint8_t)0b0001 , SWEEP = (uint8_t)0b0010 , STEP = (uint8_t)0b0100 } e_mode;

typedef struct{
	e_mode mode;
	int16_t start_point;
	int16_t end_point;
}Consigne;

typedef struct{
	int16_t speed_tachy;
	int16_t speed_encoder;
	int16_t consigne;
}Data;

static volatile struct{
	uint8_t button			: 1;
	uint8_t consigneUpdate	: 1;
	uint8_t mainProcess		: 1;
	uint8_t sendData		: 1;
} flag;


void Global_Config ( void );
void ADC_Config ( void );
void SendData( volatile Data * data );
void Data_Init( volatile Data * data );
void UpdateValues( volatile Data * data );
void Consigne_Init( Consigne * consigne );
void UpdateConsigne( Consigne * consigne );
void SysTick_Handler( void );

static volatile uint8_t t_USART3_rx_buffer[5];
static const uint8_t MODE_OFFSET = 0;
static const uint8_t SIGNE_OFFSET = 0;
static const uint8_t START_OFFSET = 1;
static const uint8_t END_OFFSET = 3;

uint16_t t_adc_buffer[2];
static const uint8_t TACHY_OFFSET = 0;
static const uint8_t POT_OFFSET = 1;

static char to_send[5];

static const uint8_t START_POINT_NEG = 0b0100;
static const uint8_t END_POINT_NEG = 0b0001;
static const uint8_t START_POINT_POS = 0b1000;
static const uint8_t END_POINT_POS = 0b0010;

static volatile Data data;
/**
* @brief Entree du programme.
* @return Rien, le int est ici pour eviter un warning GCC.
*/
int main ( void )
{
	Global_Config();
	Consigne consigne;
	Consigne_Init( &consigne );

	Data_Init( &data );

	while(1)
	{
		while ( flag.mainProcess )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_12 );
			UpdateValues( &data );

			switch( consigne.mode )
			{
				case NORMAL:
					data.consigne = consigne.start_point;
					break;
				case STEP:
					break;
				case SWEEP:
					Sweep_Consigne( consigne.start_point , consigne.end_point);
					consigne.mode = NORMAL;
					break;
			}

			flag.mainProcess = 0;
		}
		while ( flag.consigneUpdate )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_14 );
			UpdateConsigne( &consigne );
			flag.consigneUpdate = 0;
		}
		while ( flag.sendData )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_13 );
			SendData( &data );
			flag.sendData = 0;
		}
		while ( flag.button )
		{
			GPIO_ToggleBits( GPIOD , GPIO_Pin_15 );
			flag.button = 0;
		}
	}
}


void Data_Init( volatile Data * data )
{
	data -> speed_encoder	= 0;
	data -> speed_tachy		= 0;
	data -> consigne		= 0;
}


void UpdateValues( volatile Data * data )
{
	data -> speed_encoder	= 0;
	Tachy_to_RPM( t_adc_buffer[TACHY_OFFSET] , &(data->speed_tachy));

	if (GPIO_ReadInputDataBit( GPIOA , GPIO_Pin_1 ) == Bit_RESET)
	{
		data -> speed_tachy = -(data -> speed_tachy);
	}
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


void SendData( volatile Data * data )
{

	uint8_t signe = 0;
	uint16_t speed_tachy_to_send = 0;
	uint16_t speed_encoder_to_send = 0;
	
	if ((data -> speed_tachy) < 0)
	{
		signe |= START_POINT_NEG;
		speed_tachy_to_send = (uint16_t)(-(data -> speed_tachy));
	}
	else
	{
		signe |= START_POINT_POS;
		speed_tachy_to_send = (uint16_t)(data -> speed_tachy);
	}
	
	if ((data -> speed_encoder) < 0)
	{
		signe |= END_POINT_NEG;
		speed_encoder_to_send = (uint16_t)(-(data -> speed_encoder));
	}
	else
	{
		signe |= END_POINT_POS;
		speed_encoder_to_send = (uint16_t)(data -> speed_encoder);
	}

	uint16_t buffer = 0;
	to_send[ SIGNE_OFFSET ] = signe;

	buffer = speed_tachy_to_send >> 6;
	buffer |= 0b00000001;
	to_send[ START_OFFSET ] = (char)buffer & 0x00FF;

	buffer = speed_tachy_to_send << 1;
	buffer |= 0b00000001;
	to_send[ START_OFFSET + 1 ] = (char)buffer & 0x00FF;


	buffer = speed_encoder_to_send >> 6;
	buffer |= 0b00000001;
	to_send[ END_OFFSET ] = (char)buffer & 0x00FF;

	buffer = speed_encoder_to_send << 1;
	buffer |= 0b00000001;
	to_send[ END_OFFSET + 1 ] = (char)buffer & 0x00FF;
	
	my_printf( &to_send[0] );
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
}


/**
* @brief Configure les ADC pour le potentiometre et le tachymetre.
*
* @details Potentiometre	: adc_consigne	-> PB0	ADC1 channel 8
* @details Tachymetre		: adc_tachy		-> PA2	ADC1 channel 2
* @param adc_buffer Buffer pour la reception DMA
*/
void ADC_Config ( void )
{
	// Configuration de l'ADC.

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStruct;
	GPIO_InitTypeDef GPIOA_InitStruct;
	GPIO_InitTypeDef GPIOB_InitStruct;
	DMA_InitTypeDef DMA_InitStructure;

	// Demarrage des horloges de GPIOB, GPIOA et ADC1
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 , ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 , ENABLE);


	// B0 est une entree analogique, sans Pull-Up.
	GPIOB_InitStruct.GPIO_Pin	= GPIO_Pin_0;
	GPIOB_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIOB_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de B0
	GPIO_Init( GPIOB , &GPIOB_InitStruct );

	// A2 est une entree analogique, sans Pull-Up.
	GPIOA_InitStruct.GPIO_Pin	= GPIO_Pin_2;
	GPIOA_InitStruct.GPIO_Mode	= GPIO_Mode_AN;
	GPIOA_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	// Initialisation de A2
	GPIO_Init( GPIOA , &GPIOA_InitStruct );

	DMA_DeInit( DMA2_Stream0 );
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0x4001204C;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)t_adc_buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);

	DMA_Cmd( DMA2_Stream0 , ENABLE );


	// Nettoie la configuration existante des ADC.
	ADC_DeInit( );

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// L'ADC est en 12 bits, l'aquisition est declenchee manuellement
	// Les donnees sont alignees a droite, sans declenchement en externe.
	ADC_InitStruct.ADC_Resolution			= ADC_Resolution_12b;
	ADC_InitStruct.ADC_DataAlign			= ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ScanConvMode			= ENABLE;
	ADC_InitStruct.ADC_ContinuousConvMode	= ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv		= ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_NbrOfConversion		= 2;

	// Initialisation de l'ADC1 avec la structure remplie au dessus.
	ADC_Init( ADC1 , &ADC_InitStruct );

	ADC_RegularChannelConfig( ADC1 , ADC_Channel_2 ,  1 , ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig( ADC1 , ADC_Channel_8 ,  2 , ADC_SampleTime_480Cycles );

	ADC_DMARequestAfterLastTransferCmd( ADC1 , ENABLE );

	ADC_DMACmd( ADC1 , ENABLE );

	// Demarre l'ADC.
	ADC_Cmd( ADC1 , ENABLE );

	ADC_SoftwareStartConv(ADC1);
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

	while ( !flag.consigneUpdate && consigne <= fin )
	{
		Set_Consigne( consigne );
		delay_nms( 1 );
		GetTachyValue( &tachy_value );

		Tachy_to_RPM( tachy_value , &rpm_speed );

		my_printf( "%i,%i,%i" , consigne , tachy_value , rpm_speed );

		consigne++;
	}
}


void GetTachyValue ( int16_t * tachy_value )
{
	uint16_t adc_value = t_adc_buffer[TACHY_OFFSET];

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
