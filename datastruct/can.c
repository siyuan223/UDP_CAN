/*
 * can.c
 *
 *  Created on: 2024年9月24日
 *      Author: 77219
 */

#include "can.h"
#include "queue.h"
#include "stdlib.h"
#include "udpDs.h"
#include "queue.h"
#include "udp_perf_client.h"


volatile uint8_t command_code;

uint8_t udpdatagram[1400]; //是否利用内存分配，设定变数组长度较好

uint8_t udpdatagramMin[17];

//缺少指针初始化
Counter canCounter;

//定义通道包
ChnPackage package;

void CANRxCounter_init(Counter *counter){

	counter->curRxCan = 0;
	counter->canFrameRxLimit = 0;
	counter->curTxUdp = 0;
}

//udp tx related
void CANRxCounter_IncrementUdp(Counter *counter, int value) {

    counter->curTxUdp += value;
}

uint16_t CANRxCounter_GetUdpCount(Counter *counter) {

    return counter->curTxUdp;
}


// can rx related
void CANRxCounter_Increment(Counter *counter, int value) {

    counter->curRxCan += value;
}

void CANRxCounter_Decrement(Counter *counter, int value) {

    counter->curRxCan -= value;
}

void CANRxCounter_Reset(Counter *counter) {

	counter->curRxCan = 0;
	counter->canFrameRxLimit = 0;
	counter->curTxUdp = 0;
}

uint16_t CANRxCounter_GetCount(Counter *counter) {

    return counter->curRxCan;
}


//can limit related
uint16_t CANRxCounter_GetLimit(Counter *counter) {

    return counter->canFrameRxLimit;
}

void CANRxCounter_SetLimit(Counter *counter, int value) {

     counter->canFrameRxLimit = value;
}

void prepareUdpdatagram(uint8_t *lArray, uint8_t *sArray, uint16_t cnt){

	int i;

	for(i = 0; i < 17; i++){

		lArray[17*cnt+i] = sArray[i];//不用减1判断，这种条件特殊，逻辑关系有别
	}

}

//打包队列数据
int ChnPackage_EnPackage(XCan *InstancePtr, ChnPackage *package, u32 *queue)
{
	XCan *CanPtr;
	u8  *tempPtr;
	u8  i;

	//帧头
	package->head = 0x55;

	//帧通道
	CanPtr = InstancePtr;

	if(CanPtr->BaseAddress == XPAR_CAN_0_BASEADDR)//确认通道
	{
		package->CH =0x00;
	}else
	{
		package->CH = 0x01;
	}

	//帧ID
	package->id = (queue[0]&XCAN_IDR_ID1_MASK)>>XCAN_IDR_ID1_SHIFT;//数据移动

	//数据长度
	package->dlc = (queue[1]&XCAN_DLCR_DLC_MASK)>>XCAN_DLCR_DLC_SHIFT;

	//命令码
	package->code = 0x00;

	//帧数据
	tempPtr = (u8 *)&queue[2];


	//小端转换大端
	for(i = 0; i < 8; i++)
	{
		if(i < 4){
			//这个位置地址？与
			package->data[i] = (u8)*(tempPtr+3-i);

		}else{
			package->data[i] = (u8)*(tempPtr+11-i);
		}

	}


	return 0;
}

// 函数：将结构体数据转化为 uint8_t 字节数组
// 单个UDP字节长度：指定20个
//装载操作，右边数据添加到左边，符合一般直觉；反方面可以应用于特殊情况

void ChnPackage_convertToU8Array(ChnPackage *pack, uint8_t *outArray) {
    int index = 0;

    // 将每个字段按顺序放入字节数组中
    outArray[index++] = pack->head;
    outArray[index++] = pack->CH;

    // 处理 32-bit 的 id
    outArray[index++] = (uint8_t)(pack->id >> 24);
    outArray[index++] = (uint8_t)(pack->id >> 16);
    outArray[index++] = (uint8_t)(pack->id >> 8);
    outArray[index++] = (uint8_t)(pack->id);

    // 处理 16-bit 的 code
    outArray[index++] = (uint8_t)(pack->code >> 8);
    outArray[index++] = (uint8_t)(pack->code);

    // 处理 dlc
    outArray[index++] = pack->dlc;  //outrArray[8];

    // 处理 data 数组  顺序调整
    memcpy(&outArray[index], pack->data, 8);

}

void CAN_Init(void)
{
	u32 filter;
	u8 status;

	//CAN 初始化
	XCan_Initialize(&Can, 0);//空指针

	XCan_EnterMode(&Can, XCAN_MODE_CONFIG);
	XCan_SetBaudRatePrescaler(&Can, 3);
	XCan_SetBitTiming(&Can, 2, 2, 7);//2+7+3

//	XCan_AcceptFilterDisable(&Can, XCAN_AFR_UAF1_MASK);
//
//	if(XCan_IsAcceptFilterBusy(&Can) ==FALSE)
//	{
//		xil_printf("set filter\n");
//	}
//
//	//第一位为0，所以id 0x01无法接收  0x00200000
//	//认识地址变量、数据存储位置映射关系！ 这是一个新的认知机会。
//	status = XCan_AcceptFilterSet(&Can, XCAN_AFR_UAF1_MASK, 0x00200000, 0x00200000); //mask  id
//	if(status != XST_SUCCESS)
//	{
//		xil_printf("filter err\n");
//	}
//
//	//滤波器设置
//	XCan_AcceptFilterEnable(&Can, XCAN_AFR_UAF1_MASK);
//
//	//判断是否打开滤波器
//	filter = XCan_AcceptFilterGetEnabled(&Can);
//	xil_printf("filter:%d\n", filter);


	XCan_EnterMode(&Can, XCAN_MODE_NORMAL);
	//CAN中断初始化

	//中断使能XCAN_IXR_RXOK_MASK|XCAN_IXR_RXNEMP_MASK
	//XCan_InterruptEnable(&Can, XCAN_IXR_RXOK_MASK);
	XCan_InterruptDisable(&Can, XCAN_IXR_RXOK_MASK);
	XCan_InterruptDisable(&Can, XCAN_IXR_RXOFLW_MASK);


	XCan_InterruptEnable(&Can, XCAN_IXR_RXNEMP_MASK);//串口影响接收中断算是，这个逻辑可行
	XCan_InterruptEnable(&Can, XCAN_IXR_TXOK_MASK);

}

//同步协议，否则上位机、下位机以及CAN节点无法高效通信
//数据包：控制-数据功能分解
//00-00-00-00--00-00-00-00
//模式-时间-帧个数16位  暂定

void  CAN_CfgSendFrame(u32 *TxFrame, u8 *canData)
{
	u8 i;
	u32 *tempPtr = TxFrame;

	u8 *TxFramePtr;

	//接收ID
	TxFrame[0] = XCan_CreateIdValue(canData[1], 0, 0, 0, 0);

	//数据长度
	TxFrame[1] = XCan_CreateDlcValue(8);//控制数据个数，因此下方逻辑出现问题

	//转换地址数据类型
	TxFramePtr = (u8 *)(&tempPtr[2]);

	//接收UDP个数
	//udpRxCnt = canData[4]*10;//注意qt端口顺序与CAN的小端 大端顺序不同

	CANRxCounter_SetLimit(&canCounter, canData[5]<<8|canData[6]);//注意计数关系

	//填充数据,大小端转换，是个有趣编程操作
	for(i = 0; i < 8; i++)
	{
		if(i > 3)
		{
			*TxFramePtr++ = i;//如果使用内部指针，需要static型
		}else
		{
			//哪里方向错误？
			*TxFramePtr++ = canData[6 - i];//逻辑关系错误，超出指针区域;
		}

	}


}

extern CAN_Buffer *queue;
#define TIME_THRESHOLD_MS 250

/*按照策略设计函数*/
void CAN_PackUpload(void){

	static int round;
	static int reminder;
	static uint8_t arrayOrder;

	static u32 RxFrame[4];//从队列中临时取出接收CAN帧

	//判断接收队列是否空

		//提取队列数据
		CAN_Buffer_Dequeue(queue, RxFrame);

		//打包队列数据
		ChnPackage_EnPackage(&Can, &package, RxFrame);//检查pack数据

		//@@@串口显示添加的package数据
		//xil_printf("%d-%d-%d-%d-%d-%d-%d-%d\n", package.data[0],package.data[1],package.data[2],\
				package.data[3],package.data[4],package.data[5],package.data[6],package.data[7]);

		//预处理为字节数据，装填UDP数组中，进行发送准备
		ChnPackage_convertToU8Array(&package, &UdpByteData[0]);//转变为8进制字节数据	//直接使用数组名代替取地址符号

		//除余操作，重置数组装填指针
		//获取个数不用再考虑

		arrayOrder = CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt;//除余操作的巧妙

		//装填字节数据到UDP包中
		prepareUdpdatagram(&udpdatagram[0], &UdpByteData[0], arrayOrder);

		//接收UDP计数
		CANRxCounter_Increment(&canCounter, 1);//考虑设计函数操作变量，避免出现分歧

		//串口显示接收个数，利于判断数据增强规律
		//xil_printf("UdpByteDataInCnt:%d\n", CANRxCounter_GetCount(&canCounter));

		//接收达到80，即发送
		//打包发送，需要keil-sdk以及qt层都需要知道发送个数信息
		//设计udpdatagram超时直接上传udp帧，内存超时策略
		//发送条件是什么？

		//timeout模式如何添加？
		//第一个CAN帧就直接发送，所以可以减少这部分时间延时！

		//明确if动作条件，这是逻辑化结果
		if((CANRxCounter_GetCount(&canCounter) == MAX_Udp_Rx_Cnt)){//|(elapsed_time_ms >= TIME_THRESHOLD_MS)){

			//发送UDP包

			//xil_printf("transfer ok\n");

			//距离上一次发送超过timeout
			transfer_data(&udpdatagram[0], 17*CANRxCounter_GetCount(&canCounter));

			//重新计时，
			//last_received_time = get_time_ms();

			//重置UDP包
			memset(udpdatagram, 0, sizeof(udpdatagram));

			//验证进入条件判断
			//xil_printf("send udp ok\n");注释全部放在主程序比较清晰

			//重置计数状态
			CANRxCounter_Reset(&canCounter);//实时接收CAN帧个数

		}

#ifdef modTx
		//判断发送UDP包时机
		if(CANRxCounter_GetLimit(&canCounter) < MAX_Udp_Rx_Cnt) //UDP包存储80个CAN包
		{//当接收CAN包小于UDP包大小

			//@@@接收个数较小时，验证接收数据
			//xil_printf("udpRxCnt:%d", udpRxCnt);//为什么出现无法改善的情况。
			//xil_printf("UdpByteDataInCnt:%d \n", UdpByteDataInCnt);

			//判断接收个数与预期个数相同
			if(CANRxCounter_GetCount(&canCounter) == CANRxCounter_GetLimit(&canCounter)){

				//发送UDP包
				transfer_data(&udpdatagram[0], 17*CANRxCounter_GetCount(&canCounter));

				//重置UDP包
				memset(udpdatagram, 0, sizeof(udpdatagram));

				//验证进入条件判断
				//xil_printf("send udp ok\n");注释全部放在主程序比较清晰

				//重置计数状态
				CANRxCounter_Reset(&canCounter);//实时接收CAN帧个数

				command_code = 0;
			}

		}else
		{

			//常量运算，条件判断依据
			round = CANRxCounter_GetLimit(&canCounter)/MAX_Udp_Rx_Cnt; //除操作，指定UDP发送需要几轮
			reminder = CANRxCounter_GetLimit(&canCounter)%MAX_Udp_Rx_Cnt;//除余操作，最后一轮发送个数

			//当轮数小于指定数量，则装满发送
			if(CANRxCounter_GetCount(&canCounter) <= round*MAX_Udp_Rx_Cnt){

				//取余运算：装满即发送
				if(CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt == 0){

					//发送满载UDP包
					transfer_data(&udpdatagram[0], 80*17);

					//重置数组：避免原来数据污染
					memset(udpdatagram, 0, sizeof(udpdatagram));//置0操作可行

					//接收个数为400，此时这个选择区间仍然要重置计数

					//满包发送
					//计数变量，如何精确判断次数
					xil_printf("接收can包个数：%d\n", CANRxCounter_GetCount(&canCounter));//个数存在问题，为320？



				}else
				{
					//条件判断：等待过程
				}

				if((CANRxCounter_GetCount(&canCounter) == round*MAX_Udp_Rx_Cnt)&&reminder == 0x00){

					//整数边界条件，重置计数状态
					CANRxCounter_Reset(&canCounter);//中立的计数变量，容易产生计数判断逻辑错误
					command_code = 0;

				}

			}else{

				//发送can包的零头
				if(CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt ==reminder){

					//发送剩余UDP包
					transfer_data(&udpdatagram[0], reminder*17);

					//重置数组为0
					memset(udpdatagram, 0, sizeof(udpdatagram));

					//重置计数状态
					CANRxCounter_Reset(&canCounter);//中立的计数变量，容易产生计数判断逻辑错误

					command_code = 0;
				}

			}

			}
#endif
}

/*直接CAN帧上传UDP*/
//$$$queue-size问题：为什么完全相同逻辑，size取值不同
//$$$没有清除Buffer缓存，覆盖数据比较奇特

void CAN_DirectUpload(void){

	static uint8_t arrayOrder;
	uint16_t cnt;
	u64 size;

	//取出缓存CAN包变量
	u32 RxFrame[4];

	//提取队列数据 + 1000帧延时判断
	CAN_Buffer_Dequeue(queue, RxFrame);


	//打包队列数据
	ChnPackage_EnPackage(&Can, &package, RxFrame);

	//适时打印数据
	//xil_printf("%x-%x-%x-%x-%x-%x-%x-%x\n", package.data[0],package.data[1],package.data[2],\
			package.data[3],package.data[4],package.data[5],package.data[6],package.data[7]);

	//预处理为字节数据，装填UDP数组中，进行发送准备
	ChnPackage_convertToU8Array(&package, &UdpByteData[0]);//转变为8进制字节数据	//直接使用数组名代替取地址符号

#ifdef modeTx
	//除余操作，重置数组装填指针
	arrayOrder = CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt;//除余操作的巧妙 为什么是85？

	//装填字节数据到UDP包中
	prepareUdpdatagram(&udpdatagramMin[0], &UdpByteData[0], arrayOrder);
#endif

	//装填字节数据到UDP包中
	prepareUdpdatagram(&udpdatagramMin[0], &UdpByteData[0], 0);

	//发送UDP
	transfer_data(&udpdatagramMin[0], 17); //


//采用直接ECanTool发送方式
#ifdef modeTx
	//增加udp发送计数
	CANRxCounter_IncrementUdp(&canCounter, 1);

	//打印UDP发送计数变量
	cnt = CANRxCounter_GetUdpCount(&canCounter);
	xil_printf(" CANRxCounter_GetUdpCount:%x; limit:%x\n", cnt,CANRxCounter_GetLimit(&canCounter) );

	//如何保证在无法正常接收时，定量计算这种偏差，比如zynq负载率与实际DDR之间的关系。
	//为什么此if无法进入？
	if(CANRxCounter_GetUdpCount(&canCounter) == CANRxCounter_GetLimit(&canCounter)){

		command_code = 0;
		CANRxCounter_Reset(&canCounter);

		clear_CAN_Buffer(queue);
		xil_printf("udp send ok\n");
	}
#endif
}
