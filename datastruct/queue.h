/*
 * queue.h
 *
 *  Created on: 2024��9��24��
 *      Author: 77219
 */


#ifndef DATASTRUCT_QUEUE_H_
#define DATASTRUCT_QUEUE_H_

//��������DDR�����Сֱ����أ���ͨ�����϶࣬������Դ��������Σ�
#define BUFFER_SIZE 10000// 1000 �У�ÿ�д� 8 ���ֽ�  ;1.5KB����ռ��ǿ���
//��ǿ����ֵ�����Ӱ��ת��Ч�ʣ�1500 -��3000
//3000��С���棬�ܹ�����ֱ��ģʽ��
//�ڴ�ռ�ã�16n = 50000*16= 800K   1W = 1.6M
//50000->10000 can֡ʧ��ʧ����2K��

#define DATA_SIZE 4      // ÿ�� 8 �ֽ�


#include "xil_types.h"
#include "stdbool.h"

typedef struct {
	volatile  u32 buffer[BUFFER_SIZE][DATA_SIZE];  // 2D ���д洢�ռ�
    volatile  u32 head;  // ����ͷ
    volatile  u32 tail;  // ����β
    volatile  u32 size;  // ��ǰ������ ���һ��u32��Ӱ����ʲô��
   // u32 test; //���Ա���������ͻ����

} CAN_Buffer;

//���нṹ�Ͳ���
#define DDR_CANDATA_BUFFER 0x00600000


void CAN_Buffer_Init(CAN_Buffer *queue);

u64 CAN_Buffer_GetSize(CAN_Buffer *queue);

// �������Ƿ�Ϊ��
bool CAN_Buffer_IsEmpty(CAN_Buffer *queue);

// �������Ƿ�����
bool CAN_Buffer_IsFull(CAN_Buffer *queue);

// ��Ӳ��������� 8 �ֽ�����
bool CAN_Buffer_Enqueue(CAN_Buffer *queue, u32 *data);

// ���Ӳ�����ȡ�� 8 �ֽ�����
bool CAN_Buffer_Dequeue(CAN_Buffer *queue, u32 *data);

void clear_CAN_Buffer(CAN_Buffer* buffer);


#endif /* DATASTRUCT_QUEUE_H_ */
