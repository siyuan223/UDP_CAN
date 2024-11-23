/*
 * udpDs.c
 *
 *  Created on: 2024��9��26��
 *      Author: 77219
 */
#include "xil_types.h"


volatile u8 udpBuf[2500];
volatile u8 udpRxFlag; //ע��������ò���
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
