#pragma once
#include "stm32f4xx.h"

static const uint8_t SAMPLING_PERIOD = 1; // Sampling period in ms

typedef enum { NORMAL = (uint8_t)0b0001 , SWEEP = (uint8_t)0b0010 , STEP = (uint8_t)0b0100 , EXTERNAL_COMMAND = (uint8_t)0b1000 } e_mode;

typedef struct{
	e_mode mode;
	int16_t start_point;
	int16_t end_point;
}t_ConsigneReceived;

typedef struct{
	int16_t speed_tachy;
	int16_t speed_encoder;
	int16_t speed;
	int16_t consigne;
	int16_t potardValue;
}t_Data;

typedef struct{
	uint8_t button			: 1;
	uint8_t consigneUpdate	: 1;
	uint8_t potardUpdate	: 1;
	uint8_t mainProcess		: 1;
	uint8_t sendData		: 1;
	uint8_t UARTTXReady		: 1;
	uint16_t 				:10;
}t_Flag;

/**
 * @var Used for interrupt flag
 */
volatile t_Flag flag;

void Data_Init( t_Data * data );
void Consigne_Init( t_ConsigneReceived * consigne );
