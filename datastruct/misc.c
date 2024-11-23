/*
 * misc.c
 *
 *  Created on: 2024��9��26��
 *      Author: 77219
 */

#include "misc.h"
#include "xgpiops.h"
#include "xparameters.h"
#include "sleep.h"
#include "xgpio.h"

XGpioPs Gpio;
XGpio GpioPL;


//u8 checkAndResetFlag(void)
//{
//	if(udpRxFlag)
//	{
//		udpRxFlag = 0;
//
//		return 1;
//	}
//
//	return 0;
//}

//void udpISR(void)
//{
//	udpRxFlag = 1;
//}

void canISR(void)
{
	canRxFlag = 1;
}

void LED_Init(void){

	XGpioPs_Config *ConfigPtr;
	u8 Status;

	/* Initialize the GPIO driver. */
	ConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("gpio err\n");
		return XST_FAILURE;
	}

	//�޸�ֵ
	//ͻȻ��ʶ�������������޷���������ֵ��
	XGpioPs_SetDirectionPin(&Gpio, 9, 1);
	//XGpioPs_SetOutputEnable(&Gpio, 0, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, 9, 1);

}


void LED_Spakle(void){

	u32 pinState;
	u32 pinStateBank;

	//static u8 toggle;
//	pinStateBank =XGpioPs_Read(&Gpio, 0);
//	xil_printf("pinStateBank:%x\n", pinStateBank);
//
//	pinState = XGpioPs_ReadPin(&Gpio, 7);//��̴��ֲ�������Ӱ��״̬�
//	xil_printf("pinState:%x\n", pinState);
//

	/* Set the GPIO output to be low. */
	if(XGpioPs_ReadPin(&Gpio, 9) == 1){

		XGpioPs_WritePin(&Gpio, 9, 0x0);
		//xil_printf("readpin 01\n");

	}else if(XGpioPs_ReadPin(&Gpio, 9) == 0){//�����Ŷ���ϸ��
		//λ���������ö�Ӧλ�õ�״̬

		XGpioPs_WritePin(&Gpio, 9, 0x1);
		//xil_printf("readpin 00\n"); //Ϊʲôֻ��һ����ڣ�
	}
//	xil_printf("�ܹ��������ѽ\n");
//	XGpioPs_WritePin(&Gpio, 7, 0x0);//�����ļ��������⣿�����ļ���������
//	sleep(1);
//	XGpioPs_WritePin(&Gpio, 7, 0x1);
//	sleep(1);
}

void LED2_Init(void){

	//xil_printf("led2 in\n");

	XGpio_Initialize(&GpioPL, 0);

	//xil_printf("led2 s2\n");
	XGpio_SetDataDirection(&GpioPL, 1, 0);//����������⣬�м������channel������io��

	//xil_printf("led2 out\n");

}

void LED2_Spakle(void){

	u32 status;

	status = XGpio_DiscreteRead(&GpioPL, 1);
	//xil_printf("gpio pl status:%x", status);//���ڲ��Ա������Ϸ���ZYNQ���ܳ��ֵ����⡣
	//���ٴ�ӡӰ��۲�

	if(XGpio_DiscreteRead(&GpioPL, 1) == 0x1)
	{
		XGpio_DiscreteWrite(&GpioPL, 1, 0);
	}else{
		XGpio_DiscreteWrite(&GpioPL, 1, 1);
	}

}

void LED3_Init(void){

	XGpioPs_Config *ConfigPtr;
	u8 Status;

	/* Initialize the GPIO driver. */
	ConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("gpio err\n");
		return XST_FAILURE;
	}

	//�޸�ֵ
	//ͻȻ��ʶ�������������޷���������ֵ��
	XGpioPs_SetDirectionPin(&Gpio, 10, 1);
	//XGpioPs_SetOutputEnable(&Gpio, 0, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, 10, 1);

}

void LED3_twink(void){

	u32 pinState;
	u32 pinStateBank;

	//static u8 toggle;
//	pinStateBank =XGpioPs_Read(&Gpio, 0);
//	xil_printf("pinStateBank:%x\n", pinStateBank);
//
//	pinState = XGpioPs_ReadPin(&Gpio, 7);//��̴��ֲ�������Ӱ��״̬�
//	xil_printf("pinState:%x\n", pinState);
//

	/* Set the GPIO output to be low. */
	if(XGpioPs_ReadPin(&Gpio, 10) == 1){

		XGpioPs_WritePin(&Gpio, 10, 0x0);
		//xil_printf("readpin 01\n");

	}else if(XGpioPs_ReadPin(&Gpio, 10) == 0){//�����Ŷ���ϸ��
		//λ���������ö�Ӧλ�õ�״̬

		XGpioPs_WritePin(&Gpio, 10, 0x1);
		//xil_printf("readpin 00\n"); //Ϊʲôֻ��һ����ڣ�
	}
}
