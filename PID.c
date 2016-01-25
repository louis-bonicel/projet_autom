#include "PID.h"

void PID_Init( t_PID * pid )
{
	pid -> lastProcessedValue	= 0;
	pid -> error				= 0;
	pid -> sumError				= 0;
	pid -> KP					= DEFINED_KP;
	pid -> KI					= DEFINED_KI;
	pid -> KD					= DEFINED_KD;
	pid -> consigneOut			= 0;
}


void PID_Calculate( t_PID * pid , t_Data data )
{
	static float consigne_p = 0;
	static float consigne_i = 0;
	static float consigne_d = 0;
	float consigne = 0;

	pid -> error = (float) data.consigneReceived - data.speed;
	pid -> sumError += pid -> error;

	if (pid->sumError > PID_MAX_ERROR)
		pid->sumError = PID_MAX_ERROR;
	if (pid->sumError < -PID_MAX_ERROR)
		pid->sumError = -PID_MAX_ERROR;

	consigne_p = pid->KP * pid->error;
	consigne_i = ( pid->KI * pid->sumError ) + consigne_i;
	// consigne_i = ( pid->KI * pid->error ) + consigne_i;
	consigne_d = pid->KD * ( pid->lastProcessedValue - data.speed );

	consigne = (int16_t)(consigne_p + consigne_i + consigne_d);

	if ( consigne > PID_MAX_CONSIGNE )
		pid->consigneOut = PID_MAX_CONSIGNE;
	if ( consigne < -PID_MAX_CONSIGNE )
		pid->consigneOut = -PID_MAX_CONSIGNE;

	pid->lastProcessedValue = data.speed;
}