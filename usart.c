/**
* \file usart.c
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Configuration et acces au port USART.
*
* \details Ce fichier regroupe l'implementation des differentes fonctions
* qui seront necessaires pour communiquer en utilisant le protocole USART.
*
*/

#include "stm32f4xx_conf.h"
#include "usart.h"

/**
 * @brief Cette fonction configure les GPIOs utilises pour l'USART et
 * le peripherique USART3.
 *
 * @details TX -> PD8, USART 3
 * @details RX -> PD9, USART 3
 * @param rx_buffer Buffer pour la reception DMA
 */
void USART3_Config( volatile uint8_t * rx_buffer )
{
	// Structure qui sera utilisee pour initialiser les GPIOs
	GPIO_InitTypeDef GPIO_InitStructure;

	// Structure pour initialiser l'USART
	USART_InitTypeDef USART_InitStructure;

	DMA_InitTypeDef DMA_InitStructure;

	// Demarrage de l'horloge GPIOD
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD , ENABLE );

	// Demarrage de l'horloge de l'USART3
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3 , ENABLE );

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1 , ENABLE );

	// Les pins utilises sont les pins 8 et 9 ( voir schema electrique de la board )
	// L'USART est une "Alternative function"
	// Output type : PushPull
	// Pull-Up
	// Max speed !
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;

	// Initialise le port D avec les parametres precedents
	GPIO_Init( GPIOD , &GPIO_InitStructure );

	// L'USART est une fonction alternative, on parametre donc les pins
	// pour les "mapper" sur celle-ci
	GPIO_PinAFConfig( GPIOD , GPIO_PinSource8 , GPIO_AF_USART3 );
	GPIO_PinAFConfig( GPIOD , GPIO_PinSource9 , GPIO_AF_USART3 );

	// On configure le peripherique USART comme
	// Baudrate = 115200
	// 8N1
	// Pas de Flow Control
	// En Rx/Tx puisqu'il est prevu de communiquer en duplex
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	// On applique les parametres
	USART_Init( USART3 , &USART_InitStructure );


	DMA_DeInit( DMA1_Stream1 );
	DMA_StructInit( &DMA_InitStructure );

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0x40004804;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)16;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rx_buffer;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init( DMA1_Stream1 , &DMA_InitStructure );

	USART_DMACmd( USART3 , USART_DMAReq_Rx , ENABLE );

	DMA_Cmd( DMA1_Stream1 , ENABLE );

	// Et on demarre l'USART
	USART_Cmd( USART3 , ENABLE );
}
