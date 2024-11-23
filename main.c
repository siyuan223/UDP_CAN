/*
 * Copyright (C) 2017 - 2018 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "sleep.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "xil_cache.h"
#include <stdbool.h>
#include "XCAN.h"

//用户文件
#include "queue.h"
#include "can.h"
#include "udpDs.h"
#include "misc.h"

//#include "pthread.h"


//控制协议变量
u16 default_RxCnt = 0;
u16 tx_udp_cnt = 0;

//中断函数声明
static void RecvHandler( void *CallBackRef);//函数外定义类型，需要处于这个声明前面
static void SendHandler(void *CallBackRef);

//rxmod变量
volatile uint8_t rxMod;
volatile uint8_t timeout;


#if LWIP_DHCP==1
#include "lwip/dhcp.h"
extern volatile int dhcp_timoutcntr;
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

#define DEFAULT_IP_ADDRESS	"192.168.1.10"
#define DEFAULT_IP_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDRESS	"192.168.1.1"

void platform_enable_interrupts(void);
void start_application(void);
void transfer_data(uint8_t *array);//声明放在main函数中
void print_app_header(void);

#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || \
		 XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif

struct netif server_netif;

static void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP:       ", ip);
	print_ip("Netmask :       ", mask);
	print_ip("Gateway :       ", gw);
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if (!err)
		xil_printf("Invalid default IP address: %d\r\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if (!err)
		xil_printf("Invalid default IP MASK: %d\r\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if (!err)
		xil_printf("Invalid default gateway address: %d\r\n", err);
}

CAN_Buffer *queue;
int main(void)
{

	u32 RxFrameTest[4];
	u8 *RxFrameTestPtr;

	u32 RxFrame[4];

	//初始化队列指针
	queue = (CAN_Buffer *)DDR_CANDATA_BUFFER;

	//内存对齐判断
	if (((uintptr_t)queue % 4) == 0) {
	    xil_printf("queue 是 4 字节对齐的。\n");
	} else {
	    xil_printf("queue 不是 4 字节对齐的。\n");
	}

	//初始化队列
	 CAN_Buffer_Init(queue);

	 //初始化PS IO
	 LED_Init();

	 //初始化PL IO
	 LED2_Init();

	 //使用外设需要初始化
	 LED3_Init();

	 //初始化counter
	 CANRxCounter_init(&canCounter);

	 //初始化Can
	 CAN_Init();//文件引用和函数变量引用出现问题

	//配置中断函数
	XCan_SetHandler(&Can, XCAN_HANDLER_RECV, (void *)RecvHandler, (void *)&Can);//填充结构体
	XCan_SetHandler(&Can, XCAN_HANDLER_SEND, (void *)SendHandler, (void *)&Can);//注册回调函数

	//初始化网络接口结构体
	struct netif *netif;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	netif = &server_netif;
#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || \
		XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

	/* Define this board specific macro in order perform PHY reset
	 * on ZCU102
	 */
#ifdef XPS_BOARD_ZCU102
	IicPhyReset();
#endif

	init_platform();

	xil_printf("\r\n\r\n");
	xil_printf("-----lwIP RAW Mode UDP Client Application-----\r\n");

	/* initialize lwIP */
	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
				PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return -1;
	}
	netif_set_default(netif);

	/* now enable interrupts */
	platform_enable_interrupts(); //其它文件函数，作用在这里

	/* specify that the network if is up */
	netif_set_up(netif);

#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	dhcp_start(netif);
	dhcp_timoutcntr = 24;
	while (((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(netif);

	if (dhcp_timoutcntr <= 0) {
		if ((netif->ip_addr.addr) == 0) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			assign_default_ip(&(netif->ip_addr),
					&(netif->netmask), &(netif->gw));
		}
	}

	/* print IP address, netmask and gateway */
#else
	assign_default_ip(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
#endif
	print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));

	xil_printf("\r\n");

	/* print app header */
	print_app_header();

	/* start the application*/
	start_application();
	xil_printf("zynq start ok\r\n");

#ifdef testMode
	//初始化10-0000个CAN帧
	for(int i = 0; i < 100000; i++){

		RxFrameTest[0] = XCan_CreateIdValue(1, 0, 0, 0, 0);

		//数据长度
		RxFrameTest[1] = XCan_CreateDlcValue(8);//控制数据个数，因此下方逻辑出现问题

		//转换地址数据类型
		RxFrameTestPtr = (u8 *)(&RxFrameTest[2]);

		//
		*(RxFrameTestPtr + 0) = (i / 1000) % 10; // 千位
		*(RxFrameTestPtr + 1) = (i / 10000) % 10;   // 万位
		*(RxFrameTestPtr + 2) = (i / 100000) % 10; // 十万位
		*(RxFrameTestPtr + 3) = 'A';
		*(RxFrameTestPtr + 4) = 'A';
		*(RxFrameTestPtr + 5) = i % 10;
		*(RxFrameTestPtr + 6) = (i / 10) % 10;
		*(RxFrameTestPtr + 7) = (i / 100) % 10;   // 百位

		//入队CAN帧
		CAN_Buffer_Enqueue(queue, RxFrameTest);
	}

#endif

	while (1) {

		/*获取网络接口数据*/
		xemacif_input(netif);

//		sleep(1);
//		xil_printf("test\n");

		/*下行UDP->CAN*/
		if(checkAndResetFlag())
		{		//判断UDP帧接收中断状态

			//配置CAN发送帧
			CAN_CfgSendFrame(TxFrame, udpBuf);

			//get command code，mention code location
			command_code = udpBuf[3];

			//transmit CAN帧
			XCan_Send(&Can, TxFrame);

		}

		/*upload CAN->UDP*/
		if(!CAN_Buffer_IsEmpty(queue)){


//发送命令控制发送模式
#ifdef modeTx
			//decide upload mode
			switch(command_code){

				case 1:
					//xil_printf("case1 ok\n");//
					CAN_DirectUpload();

					break;
				case 2:
					//xil_printf("case2 ok\n");
					CAN_PackUpload();

					break;
				default:
					break;

			}
#else
			//暂时设定为1
			if(rxMod == 1){

				//提取队列数据 + 1000帧延时判断
				//直接打印数据，不发送，测量接收延时时间
				//此函数取出即可，不用特地清除！这个变化，增加了这个函数的特性认识
				//CAN_Buffer_Dequeue(queue, RxFrame);

				CAN_DirectUpload();
				//tx_udp_cnt++;

				//可能丢失CAN帧没有考虑，后期添加
				//if(default_RxCnt == (tx_udp_cnt - 1)){
				LED_Spakle();//红色

				//	tx_udp_cnt = 0;
				//	default_RxCnt = 0;
				//}



			}else if(rxMod == 3){


				CAN_PackUpload();


			}else if(rxMod == 5){

				//清除判断
				xil_printf("clear buffer\n");

				clear_CAN_Buffer(queue);
				rxMod = 0;
			}


#endif

		}

	/* never reached */
	//cleanup_platform();

	//return 0;
}

}//括号的编译器识别有问题/


/*CAN接收中断函数*/
//两种中断不同，需要分别处理，保证比较好的可靠性。
//精简中断逻辑，只保留核心接收

//00-00-00 当前发送CAN帧个数
//00-00-00 指定接收个数
//00-00 设定模式，代定

static void RecvHandler( void *CallBackRef)
{
	volatile static int cnt;
	int status;

	int intrStatus;
	u32 RxFrame[4];//局部变量定义在对应函数

	u8 *RxFramePtr;
	u16 RxCnt;


	//获取回调指针
	XCan *CanPtr = (XCan *)CallBackRef;

	//中断类型判断

	if(XCan_InterruptGetStatus(CanPtr) == XCAN_IXR_RXNEMP_MASK){

		//接收CAN帧
		status = XCan_Recv(CanPtr, RxFrame);

		if(RxFrame[0] == XCan_CreateIdValue(1, 0, 0, 0, 0)){

			rxMod = 1;

		}else if(RxFrame[0] == XCan_CreateIdValue(3, 0, 0, 0, 0)){

			rxMod = 3;
			//xil_printf("mod3\n");

		}else if(RxFrame[0] == XCan_CreateIdValue(5, 0, 0, 0, 0)){

			xil_printf("mod5\n");
			rxMod = 5;
		}

		//获取当前CAN帧接收个数
		RxFramePtr = (u8 *)(&RxFrame[2]);
		RxCnt = (*(RxFramePtr+2)<<8)|(*(RxFramePtr+3));

		//xil_printf("RxCnt:%x", RxCnt);

		//获取默认接收CAN帧个数
		if(RxCnt == 0){

			default_RxCnt = (*(RxFramePtr+7)<<8)|(*(RxFramePtr+6));
			xil_printf("default_RxCnt:%x", default_RxCnt);

		}else if(default_RxCnt ==RxCnt ){

			//时间戳
			LED3_twink(); //棕色

			RxCnt = 0;

		}



		//队列满提醒
		if(CAN_Buffer_IsFull(queue) == 1){

			xil_printf("can buffer full\n");
		}

		//入队CAN帧

		//通过解析数据，可以进入不同入队操作
		//CAN_buffer_directEnqueue(queue, RxFrame);
		//CAN_buffer_packetEnqueue(queue, RxFrame);



	/*	while (!XCan_IsRxFifoEmpty(CanPtr)) {
					ProcessCanFrame(CanInstance);
				}*/
		CAN_Buffer_Enqueue(queue, RxFrame);

		//添加中断，判断为什么无法进入中断；
		//猜想可能是进入某个while循环
		//
//		switch(mode){
//
//		case XCAN_IXR_RXOFLW_MASK:
//			xil_printf("#1\n");
//			break;
//		case XCAN_IXR_BSOFF_MASK :
//			xil_printf("#2\n");
//			break;
//
//		}

		//添加直接计数变量
		//可能解释是，打印功能延时较高，影响中断性能，因此需要的是数字分析
		//可以直接查看变量，是否影响这个过程，并且调整

//		cnt++;
//		//xil_printf("CNT[%d]", cnt);
//		xil_printf("cnt:[%d]+++", cnt);

		//刚好八个，说明这个中断函数是没有问题
		//接收1000个CAN帧 大约是1s
		//再围绕自己要做的事情，重新编程实现

//		if(cnt%10 == 0){
//
//			LED3_twink();
//		}
//
//		if(cnt == 8000){
//
//			cnt = 0;
//		}


		//判断是否进入中断
		if(status == XST_SUCCESS)
		{
			//xil_printf("can recv\r\n");
		}

		//can中断置位
		canISR();

		//中断进入，判断与清除中断标志位，必要
		//仍然是1613个，其它CAN帧去哪里，为什么CAN帧无法及时接收，其它情况下就可以
		//XCan_InterruptClear(CanPtr, XCAN_IXR_RXNEMP_MASK);
		//XCan_InterruptClear(CanPtr, XCAN_IXR_RXOK_MASK);

		//return ;
	}


	//xil_printf("other intr\n");

	return ;
}

/*CAN发送中断函数*/
static void SendHandler(void *CallBackRef)
{
	//发送成功，串口发送状态
	xil_printf("send ok\r\n");

	return ;
}

