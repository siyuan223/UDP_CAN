/*
 * udpDs.c
 *
 *  Created on: 2024年9月26日
 *      Author: 77219
 */
#include "xil_types.h"


volatile u8 udpBuf[2500];
volatile u8 udpRxFlag; //注意接收重置操作
volatile u16 udpCount;


u8 checkAndResetFlag(void)
{
	if(udpRxFlag)
	{
		udpRxFlag = 0;

		return 1;
	}

	return 0;
}

void udpISR(void)
{
	udpRxFlag = 1;
}
