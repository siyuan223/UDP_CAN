/*
 * can.h
 *
 *  Created on: 2024��9��24��
 *      Author: 77219
 */

#ifndef DATASTRUCT_CAN_H_
#define DATASTRUCT_CAN_H_

#include "xcan.h"

//��ʼ���û�����
XCan Can;

//����CAN֡
u32 TxFrame[4];

//udp�ֽ�����
uint8_t UdpByteData[17];


#define MAX_Udp_Rx_Cnt 80 // udp1472

extern uint8_t udpdatagram[1400];


extern volatile uint8_t command_code;


//����volatile�ᵼ�±���ͻ�䣬�����ڴ������������⵼�±�������
typedef struct{
	 uint64_t curRxCan;//��ǰ����CAN֡����

	 uint64_t curTxUdp;
	 uint64_t canFrameRxLimit;//can����ָ֡�����ޣ�qt������
}Counter;

//ȱ��ָ���ʼ��
extern Counter canCounter;

//����ͨ��������
typedef struct{
	//��ַ������ʶ��ǿ
	uint8_t head;  //֡ͷ�ڽ��ղ����У�Ч���ȽϺ�
	uint8_t CH;
	uint32_t id;  //4�ֽ�
	uint16_t code; //2�ֽ�
	uint8_t  dlc;
	uint8_t data[8];
}ChnPackage;

//����ͨ����
extern ChnPackage package;


//���CAN��������
int ChnPackage_EnPackage(XCan *InstancePtr, ChnPackage *package, u32 *queue);

//ת��ΪU8����
void ChnPackage_convertToU8Array(ChnPackage *pack, uint8_t *outArray);

//����CAN֡
void  CAN_CfgSendFrame(u32 *TxFrame, u8 *canData);

//����CAN
void CAN_Init(void);

//������ȫ�����ݣ�ȫ����ӵ�������������
void prepareUdpdatagram(uint8_t *lArray, uint8_t *sArray, uint16_t cnt);

//ֱ���ϴ�CAN��
void CAN_DirectUpload(void);

//����ϴ�CAN��
void CAN_PackUpload(void);



void CANRxCounter_init(Counter *counter);

//����CAN֡���ռ���
void CANRxCounter_Increment(Counter *counter, int value);
void CANRxCounter_Decrement(Counter *counter, int value);

void CANRxCounter_Reset(Counter *counter);
uint16_t CANRxCounter_GetCount(Counter *counter);
uint16_t CANRxCounter_GetLimit(Counter *counter);

//����can֡limit����
void CANRxCounter_SetLimit(Counter *counter, int value);

//udp�������
void CANRxCounter_IncrementUdp(Counter *counter, int value);

uint16_t CANRxCounter_GetUdpCount(Counter *counter);


//void CANRxCounter_ResetLimit(Counter *counter );


#endif /* DATASTRUCT_CAN_H_ */
