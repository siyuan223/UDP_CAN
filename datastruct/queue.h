/*
 * queue.h
 *
 *  Created on: 2024年9月24日
 *      Author: 77219
 */


#ifndef DATASTRUCT_QUEUE_H_
#define DATASTRUCT_QUEUE_H_

//负载率与DDR缓存大小直接相关，当通道数较多，缓存资源消耗量如何？
#define BUFFER_SIZE 10000// 1000 行，每行存 8 个字节  ;1.5KB缓存空间是可行
//增强此数值，如何影响转换效率？1500 -》3000
//3000大小缓存，能够缓存直发模式。
//内存占用：16n = 50000*16= 800K   1W = 1.6M
//50000->10000 can帧失丢失率有2K？

#define DATA_SIZE 4      // 每行 8 字节


#include "xil_types.h"
#include "stdbool.h"

typedef struct {
	volatile  u32 buffer[BUFFER_SIZE][DATA_SIZE];  // 2D 队列存储空间
    volatile  u32 head;  // 队列头
    volatile  u32 tail;  // 队列尾
    volatile  u32 size;  // 当前数据量 添加一个u32，影响是什么？
   // u32 test; //测试变量竞争冲突问题

} CAN_Buffer;

//队列结构和操作
#define DDR_CANDATA_BUFFER 0x00600000


void CAN_Buffer_Init(CAN_Buffer *queue);

u64 CAN_Buffer_GetSize(CAN_Buffer *queue);

// 检查队列是否为空
bool CAN_Buffer_IsEmpty(CAN_Buffer *queue);

// 检查队列是否已满
bool CAN_Buffer_IsFull(CAN_Buffer *queue);

// 入队操作，插入 8 字节数据
bool CAN_Buffer_Enqueue(CAN_Buffer *queue, u32 *data);

// 出队操作，取出 8 字节数据
bool CAN_Buffer_Dequeue(CAN_Buffer *queue, u32 *data);

void clear_CAN_Buffer(CAN_Buffer* buffer);


#endif /* DATASTRUCT_QUEUE_H_ */
