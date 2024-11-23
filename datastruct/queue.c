/*
 * queue.c
 *
 *  Created on: 2024年9月24日
 *      Author: 77219
 */

#include "queue.h"
#include "xil_types.h"
#include <stdbool.h>
#include "xtime_l.h"

// 初始化队列
void CAN_Buffer_Init(CAN_Buffer *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

// 检查队列是否为空
bool CAN_Buffer_IsEmpty(CAN_Buffer *queue) {
    return queue->size == 0;
}

// 检查队列是否已满
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

// 入队操作，插入 8 字节数据
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

        return false;  // 队列已满，无法插入
    }

    // 插入 8 字节数据到队列尾部
    for (int i = 0; i < DATA_SIZE; i++) {
        queue->buffer[queue->tail][i] = data[i];
    }

    // 更新队列尾部位置
    queue->tail = (queue->tail + 1) % BUFFER_SIZE;  // 环形队列
    queue->size++;  // 更新当前队列大小

    //读取与加一行为不同，读取行为直接导致与中断中的加一行为产生冲突？
    //即避免中断和main同时处理queue

    //队列添加变量
//    queue->test +=1;
//    xil_printf("test:[%d]", queue->test);

//    if(queue->test ==1000){
//
//    	queue->test =0;
//    }


    //串口计数跳跃
    //所有size应该都能接收才对，但是现在只能处理一部分，什么原因造成？
    //这个代码问题是什么？

    //添加下面代码，首先size只打印部分，另外添加gettime，则直接无法接收CAN帧
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



// 出队操作，取出 8 字节数据
bool CAN_Buffer_Dequeue(CAN_Buffer *queue, u32 *data) {

	u64 cnt;

    if (CAN_Buffer_IsEmpty(queue)) {
        return false;  // 队列为空，无法取出
    }

    // 取出 8 字节数据
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = queue->buffer[queue->head][i];
    }

    // 更新队列头部位置
    queue->head = (queue->head + 1) % BUFFER_SIZE;  // 环形队列
    queue->size--;  // 更新当前队列大小

    //计时变化频率，计数变化个数
//    cnt++;
//
//    //提前获得队列值
//    //指示意义是否充足
//    if(cnt == 15){
//    	xil_printf("---QUEUESize:[%d]", queue->size);
//    	xil_printf("\n\n");
//    	cnt = 0;
//
//    }else{
//
//    	xil_printf("---QUEUESize:[%d]", queue->size);
//    }

    //在自动发送模式下，如何计量延时

	//sizeCnt = queue->size;
//竞争冲突场景
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

#include <string.h>  // 用于 memset 函数
void clear_CAN_Buffer(CAN_Buffer* buffer) {
    // 将头、尾指针和当前大小重置为初始状态
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;

    // 可选：如果需要清除存储的数据，可以用 memset 将 buffer 清零
    memset(buffer->buffer, 0, sizeof(buffer->buffer));
}
