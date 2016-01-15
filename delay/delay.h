/**
 * \file delay.h
 *
 * \author Inconnu
 *
 * \brief Decalaration des fonctions pour la creation de delay par le SysTick.
 *
 */


#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f4xx.h"

void SysTick_Init(void);
void TimeTick_Decrement(void);
void delay_nus(u32 n);
void delay_1ms(void);
void delay_nms(u32 n);

#endif // __DELAY_H
