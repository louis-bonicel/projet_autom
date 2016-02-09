#pragma once

#include "stm32f4xx.h"
#include "PrjDefinitions.h"

typedef struct{
	int16_t lastProcessedValue;
	float error;
	float sumError;
	float KP;
	float KI;
	float KD;
	int16_t consigneOut;
}t_PID;

static const float DEFINED_KP = 20;
static const float DEFINED_KI = 0.05;
static const float DEFINED_KD = 1;

static const int16_t PID_MAX_ERROR = 8000;
static const int16_t PID_MAX_CONSIGNE = 8000;

void PID_Init( t_PID * pid );
void PID_Calculate( t_PID * pid , t_Data * data );
