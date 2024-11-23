/*
 * queue.c
 *
 *  Created on: 2024��9��24��
 *      Author: 77219
 */

#include "queue.h"
#include "xil_types.h"
#include <stdbool.h>
#include "xtime_l.h"

// ��ʼ������
void CAN_Buffer_Init(CAN_Buffer *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

// �������Ƿ�Ϊ��
bool CAN_Buffer_IsEmpty(CAN_Buffer *queue) {
    return queue->size == 0;
}

// �������Ƿ�����
bool CAN_Buffer_IsFull(CAN_Buffer *queue) {
    return queue->size == BUFFER_SIZE;
}

u64 CAN_Buffer_GetSize(CAN_Buffer *queue){

	return queue->size;

}

XTime last_received_time;
u64 elapsed_time_ms;
XTime current_time;
#define COUNTS_PER_MILLI_SECOND (COUNTS_PER_SECOND/1000)

// ��Ӳ��������� 8 �ֽ�����
bool CAN_Buffer_Enqueue(CAN_Buffer *queue, u32 *data) {

//	static int cnt;
//
//	cnt++;

	//xil_printf("cnt:[%d]", cnt);

//	if(cnt == 8000){
//
//		cnt = 0;
//	}

    if (CAN_Buffer_IsFull(queue)) {

        return false;  // �����������޷�����
    }

    // ���� 8 �ֽ����ݵ�����β��
    for (int i = 0; i < DATA_SIZE; i++) {
        queue->buffer[queue->tail][i] = data[i];
    }

    // ���¶���β��λ��
    queue->tail = (queue->tail + 1) % BUFFER_SIZE;  // ���ζ���
    queue->size++;  // ���µ�ǰ���д�С

    //��ȡ���һ��Ϊ��ͬ����ȡ��Ϊֱ�ӵ������ж��еļ�һ��Ϊ������ͻ��
    //�������жϺ�mainͬʱ����queue

    //������ӱ���
//    queue->test +=1;
//    xil_printf("test:[%d]", queue->test);

//    if(queue->test ==1000){
//
//    	queue->test =0;
//    }


    //���ڼ�����Ծ
    //����sizeӦ�ö��ܽ��ղŶԣ���������ֻ�ܴ���һ���֣�ʲôԭ����ɣ�
    //�������������ʲô��

    //���������룬����sizeֻ��ӡ���֣��������gettime����ֱ���޷�����CAN֡
    //xil_printf("queuesize:[%d]+++",queue->size);
    //xil_printf("queuesize:[%d]+++",queue->tail);
    //xil_printf("queuesize:[%d]+++",queue->head);

//	if( queue->size%100 == 0){
////
//
//		XTime_GetTime(&current_time);
//
//		elapsed_time_ms = (current_time - last_received_time)/COUNTS_PER_MILLI_SECOND;
//
//		last_received_time = current_time;
////
////		LED_Spakle();
////
////		xil_printf("\n\n");
//		xil_printf("queue->size:%d ;elapsed time:%d\n\n", queue->size, elapsed_time_ms);
//	}


    return true;
}

#include "xtime_l.h"



// ���Ӳ�����ȡ�� 8 �ֽ�����
bool CAN_Buffer_Dequeue(CAN_Buffer *queue, u32 *data) {

	u64 cnt;

    if (CAN_Buffer_IsEmpty(queue)) {
        return false;  // ����Ϊ�գ��޷�ȡ��
    }

    // ȡ�� 8 �ֽ�����
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = queue->buffer[queue->head][i];
    }

    // ���¶���ͷ��λ��
    queue->head = (queue->head + 1) % BUFFER_SIZE;  // ���ζ���
    queue->size--;  // ���µ�ǰ���д�С

    //��ʱ�仯Ƶ�ʣ������仯����
//    cnt++;
//
//    //��ǰ��ö���ֵ
//    //ָʾ�����Ƿ����
//    if(cnt == 15){
//    	xil_printf("---QUEUESize:[%d]", queue->size);
//    	xil_printf("\n\n");
//    	cnt = 0;
//
//    }else{
//
//    	xil_printf("---QUEUESize:[%d]", queue->size);
//    }

    //���Զ�����ģʽ�£���μ�����ʱ

	//sizeCnt = queue->size;
//������ͻ����
//    if( sizeCnt%100 == 0){
//
//    	XTime_GetTime(&current_time);
//
//    	elapsed_time_ms = (current_time - last_received_time)/COUNTS_PER_MILLI_SECOND;
//
//    	last_received_time = current_time;
//
//    	LED_Spakle();
//
//    	xil_printf("\n\n");
//    	xil_printf("queue->size:%d ;elapsed time:%d\n\n", sizeCnt, elapsed_time_ms);
//    }

    return true;
}

#include <string.h>  // ���� memset ����
void clear_CAN_Buffer(CAN_Buffer* buffer) {
    // ��ͷ��βָ��͵�ǰ��С����Ϊ��ʼ״̬
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;

    // ��ѡ�������Ҫ����洢�����ݣ������� memset �� buffer ����
    memset(buffer->buffer, 0, sizeof(buffer->buffer));
}
