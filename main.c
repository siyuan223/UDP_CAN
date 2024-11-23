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

//�û��ļ�
#include "queue.h"
#include "can.h"
#include "udpDs.h"
#include "misc.h"

//#include "pthread.h"


//����Э�����
u16 default_RxCnt = 0;
u16 tx_udp_cnt = 0;

//�жϺ�������
static void RecvHandler( void *CallBackRef);//�����ⶨ�����ͣ���Ҫ�����������ǰ��
static void SendHandler(void *CallBackRef);

//rxmod����
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
void transfer_data(uint8_t *array);//��������main������
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

	//��ʼ������ָ��
	queue = (CAN_Buffer *)DDR_CANDATA_BUFFER;

	//�ڴ�����ж�
	if (((uintptr_t)queue % 4) == 0) {
	    xil_printf("queue �� 4 �ֽڶ���ġ�\n");
	} else {
	    xil_printf("queue ���� 4 �ֽڶ���ġ�\n");
	}

	//��ʼ������
	 CAN_Buffer_Init(queue);

	 //��ʼ��PS IO
	 LED_Init();

	 //��ʼ��PL IO
	 LED2_Init();

	 //ʹ��������Ҫ��ʼ��
	 LED3_Init();

	 //��ʼ��counter
	 CANRxCounter_init(&canCounter);

	 //��ʼ��Can
	 CAN_Init();//�ļ����úͺ����������ó�������

	//�����жϺ���
	XCan_SetHandler(&Can, XCAN_HANDLER_RECV, (void *)RecvHandler, (void *)&Can);//���ṹ��
	XCan_SetHandler(&Can, XCAN_HANDLER_SEND, (void *)SendHandler, (void *)&Can);//ע��ص�����

	//��ʼ������ӿڽṹ��
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
	platform_enable_interrupts(); //�����ļ�����������������

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
	//��ʼ��10-0000��CAN֡
	for(int i = 0; i < 100000; i++){

		RxFrameTest[0] = XCan_CreateIdValue(1, 0, 0, 0, 0);

		//���ݳ���
		RxFrameTest[1] = XCan_CreateDlcValue(8);//�������ݸ���������·��߼���������

		//ת����ַ��������
		RxFrameTestPtr = (u8 *)(&RxFrameTest[2]);

		//
		*(RxFrameTestPtr + 0) = (i / 1000) % 10; // ǧλ
		*(RxFrameTestPtr + 1) = (i / 10000) % 10;   // ��λ
		*(RxFrameTestPtr + 2) = (i / 100000) % 10; // ʮ��λ
		*(RxFrameTestPtr + 3) = 'A';
		*(RxFrameTestPtr + 4) = 'A';
		*(RxFrameTestPtr + 5) = i % 10;
		*(RxFrameTestPtr + 6) = (i / 10) % 10;
		*(RxFrameTestPtr + 7) = (i / 100) % 10;   // ��λ

		//���CAN֡
		CAN_Buffer_Enqueue(queue, RxFrameTest);
	}

#endif

	while (1) {

		/*��ȡ����ӿ�����*/
		xemacif_input(netif);

//		sleep(1);
//		xil_printf("test\n");

		/*����UDP->CAN*/
		if(checkAndResetFlag())
		{		//�ж�UDP֡�����ж�״̬

			//����CAN����֡
			CAN_CfgSendFrame(TxFrame, udpBuf);

			//get command code��mention code location
			command_code = udpBuf[3];

			//transmit CAN֡
			XCan_Send(&Can, TxFrame);

		}

		/*upload CAN->UDP*/
		if(!CAN_Buffer_IsEmpty(queue)){


//����������Ʒ���ģʽ
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
			//��ʱ�趨Ϊ1
			if(rxMod == 1){

				//��ȡ�������� + 1000֡��ʱ�ж�
				//ֱ�Ӵ�ӡ���ݣ������ͣ�����������ʱʱ��
				//�˺���ȡ�����ɣ������ص����������仯�����������������������ʶ
				//CAN_Buffer_Dequeue(queue, RxFrame);

				CAN_DirectUpload();
				//tx_udp_cnt++;

				//���ܶ�ʧCAN֡û�п��ǣ��������
				//if(default_RxCnt == (tx_udp_cnt - 1)){
				LED_Spakle();//��ɫ

				//	tx_udp_cnt = 0;
				//	default_RxCnt = 0;
				//}



			}else if(rxMod == 3){


				CAN_PackUpload();


			}else if(rxMod == 5){

				//����ж�
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

}//���ŵı�����ʶ��������/


/*CAN�����жϺ���*/
//�����жϲ�ͬ����Ҫ�ֱ�����֤�ȽϺõĿɿ��ԡ�
//�����ж��߼���ֻ�������Ľ���

//00-00-00 ��ǰ����CAN֡����
//00-00-00 ָ�����ո���
//00-00 �趨ģʽ������

static void RecvHandler( void *CallBackRef)
{
	volatile static int cnt;
	int status;

	int intrStatus;
	u32 RxFrame[4];//�ֲ����������ڶ�Ӧ����

	u8 *RxFramePtr;
	u16 RxCnt;


	//��ȡ�ص�ָ��
	XCan *CanPtr = (XCan *)CallBackRef;

	//�ж������ж�

	if(XCan_InterruptGetStatus(CanPtr) == XCAN_IXR_RXNEMP_MASK){

		//����CAN֡
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

		//��ȡ��ǰCAN֡���ո���
		RxFramePtr = (u8 *)(&RxFrame[2]);
		RxCnt = (*(RxFramePtr+2)<<8)|(*(RxFramePtr+3));

		//xil_printf("RxCnt:%x", RxCnt);

		//��ȡĬ�Ͻ���CAN֡����
		if(RxCnt == 0){

			default_RxCnt = (*(RxFramePtr+7)<<8)|(*(RxFramePtr+6));
			xil_printf("default_RxCnt:%x", default_RxCnt);

		}else if(default_RxCnt ==RxCnt ){

			//ʱ���
			LED3_twink(); //��ɫ

			RxCnt = 0;

		}



		//����������
		if(CAN_Buffer_IsFull(queue) == 1){

			xil_printf("can buffer full\n");
		}

		//���CAN֡

		//ͨ���������ݣ����Խ��벻ͬ��Ӳ���
		//CAN_buffer_directEnqueue(queue, RxFrame);
		//CAN_buffer_packetEnqueue(queue, RxFrame);



	/*	while (!XCan_IsRxFifoEmpty(CanPtr)) {
					ProcessCanFrame(CanInstance);
				}*/
		CAN_Buffer_Enqueue(queue, RxFrame);

		//����жϣ��ж�Ϊʲô�޷������жϣ�
		//��������ǽ���ĳ��whileѭ��
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

		//���ֱ�Ӽ�������
		//���ܽ����ǣ���ӡ������ʱ�ϸߣ�Ӱ���ж����ܣ������Ҫ�������ַ���
		//����ֱ�Ӳ鿴�������Ƿ�Ӱ��������̣����ҵ���

//		cnt++;
//		//xil_printf("CNT[%d]", cnt);
//		xil_printf("cnt:[%d]+++", cnt);

		//�պð˸���˵������жϺ�����û������
		//����1000��CAN֡ ��Լ��1s
		//��Χ���Լ�Ҫ�������飬���±��ʵ��

//		if(cnt%10 == 0){
//
//			LED3_twink();
//		}
//
//		if(cnt == 8000){
//
//			cnt = 0;
//		}


		//�ж��Ƿ�����ж�
		if(status == XST_SUCCESS)
		{
			//xil_printf("can recv\r\n");
		}

		//can�ж���λ
		canISR();

		//�жϽ��룬�ж�������жϱ�־λ����Ҫ
		//��Ȼ��1613��������CAN֡ȥ���ΪʲôCAN֡�޷���ʱ���գ���������¾Ϳ���
		//XCan_InterruptClear(CanPtr, XCAN_IXR_RXNEMP_MASK);
		//XCan_InterruptClear(CanPtr, XCAN_IXR_RXOK_MASK);

		//return ;
	}


	//xil_printf("other intr\n");

	return ;
}

/*CAN�����жϺ���*/
static void SendHandler(void *CallBackRef)
{
	//���ͳɹ������ڷ���״̬
	xil_printf("send ok\r\n");

	return ;
}

