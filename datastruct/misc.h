/*
 * misc.h
 *
 *  Created on: 2024年9月26日
 *      Author: 77219
 */

#ifndef DATASTRUCT_MISC_H_
#define DATASTRUCT_MISC_H_

#include "xbasic_types.h"
#include "xgpiops.h"
#include "xgpio.h"

//中断全局变量
//volatile u8 udpRxFlag; //同步较慢，先修改其它变量
volatile u8 canRxFlag;
extern XGpioPs Gpio;
extern XGpio GpioPL;


//自动重置中断标志
u8 checkAndResetFlag(void );

//中断服务置位
//void udpISR(void);
void canISR(void);

void LED_Init(void);
void LED_Spakle(void);

void LED2_Init(void);
void LED2_Spakle(void);

void LED3_Init(void);
void LED3_twink(void);

#endif /* DATASTRUCT_MISC_H_ */
