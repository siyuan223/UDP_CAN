/*
 * can.h
 *
 *  Created on: 2024年9月24日
 *      Author: 77219
 */

#ifndef DATASTRUCT_CAN_H_
#define DATASTRUCT_CAN_H_

#include "xcan.h"

//初始化用户数据
XCan Can;

//发送CAN帧
u32 TxFrame[4];

//udp字节数据
uint8_t UdpByteData[17];


#define MAX_Udp_Rx_Cnt 80 // udp1472

extern uint8_t udpdatagram[1400];


extern volatile uint8_t command_code;


//猜想volatile会导致变量突变，或者内存下载区域问题导致变量错误
typedef struct{
	 uint64_t curRxCan;//当前接收CAN帧个数

	 uint64_t curTxUdp;
	 uint64_t canFrameRxLimit;//can接收帧指定上限，qt端设置
}Counter;

//缺少指针初始化
extern Counter canCounter;

//定义通道包数据
typedef struct{
	//地址对齐认识加强
	uint8_t head;  //帧头在接收测试中，效果比较好
	uint8_t CH;
	uint32_t id;  //4字节
	uint16_t code; //2字节
	uint8_t  dlc;
	uint8_t data[8];
}ChnPackage;

//定义通道包
extern ChnPackage package;


//打包CAN队列数据
int ChnPackage_EnPackage(XCan *InstancePtr, ChnPackage *package, u32 *queue);

//转变为U8数组
void ChnPackage_convertToU8Array(ChnPackage *pack, uint8_t *outArray);

//发送CAN帧
void  CAN_CfgSendFrame(u32 *TxFrame, u8 *canData);

//发送CAN
void CAN_Init(void);

//短数组全部数据，全部添加到长数据数据中
void prepareUdpdatagram(uint8_t *lArray, uint8_t *sArray, uint16_t cnt);

//直接上传CAN包
void CAN_DirectUpload(void);

//打包上传CAN包
void CAN_PackUpload(void);



void CANRxCounter_init(Counter *counter);

//设置CAN帧接收计数
void CANRxCounter_Increment(Counter *counter, int value);
void CANRxCounter_Decrement(Counter *counter, int value);

void CANRxCounter_Reset(Counter *counter);
uint16_t CANRxCounter_GetCount(Counter *counter);
uint16_t CANRxCounter_GetLimit(Counter *counter);

//设置can帧limit计数
void CANRxCounter_SetLimit(Counter *counter, int value);

//udp接收相关
void CANRxCounter_IncrementUdp(Counter *counter, int value);

uint16_t CANRxCounter_GetUdpCount(Counter *counter);


//void CANRxCounter_ResetLimit(Counter *counter );


#endif /* DATASTRUCT_CAN_H_ */
