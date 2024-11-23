/*
 * misc.h
 *
 *  Created on: 2024��9��26��
 *      Author: 77219
 */

#ifndef DATASTRUCT_MISC_H_
#define DATASTRUCT_MISC_H_

#include "xbasic_types.h"
#include "xgpiops.h"
#include "xgpio.h"

//�ж�ȫ�ֱ���
//volatile u8 udpRxFlag; //ͬ�����������޸���������
volatile u8 canRxFlag;
extern XGpioPs Gpio;
extern XGpio GpioPL;


//�Զ������жϱ�־
u8 checkAndResetFlag(void );

//�жϷ�����λ
//void udpISR(void);
void canISR(void);

void LED_Init(void);
void LED_Spakle(void);

void LED2_Init(void);
void LED2_Spakle(void);

void LED3_Init(void);
void LED3_twink(void);

#endif /* DATASTRUCT_MISC_H_ */
