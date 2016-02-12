/**
* \file asservissement.h
* \author Louis Bonicel
* \author Florian Boyer
* \author Victor Marnas
*
* \date 15 Janvier 2016
*
* \brief Declaration des fonctions pour l'asservissement
*
* \details
*
*/

#pragma once

static const uint16_t STEP_HOLD_START = 1000;
static const uint16_t STEP_HOLD_STOP = 1000;

void TIM2_Init( void );
void TIM3_Init( void );
void UpdateValues( t_Data * data );
void Correcteur( int16_t * consigne_output , t_Data * data );
void Sweep_Consigne ( int16_t consigne_min , int16_t consigne_max );
