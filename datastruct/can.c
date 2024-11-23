/*
 * can.c
 *
 *  Created on: 2024��9��24��
 *      Author: 77219
 */

#include "can.h"
#include "queue.h"
#include "stdlib.h"
#include "udpDs.h"
#include "queue.h"
#include "udp_perf_client.h"


volatile uint8_t command_code;

uint8_t udpdatagram[1400]; //�Ƿ������ڴ���䣬�趨�����鳤�ȽϺ�

uint8_t udpdatagramMin[17];

//ȱ��ָ���ʼ��
Counter canCounter;

//����ͨ����
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

		lArray[17*cnt+i] = sArray[i];//���ü�1�жϣ������������⣬�߼���ϵ�б�
	}

}

//�����������
int ChnPackage_EnPackage(XCan *InstancePtr, ChnPackage *package, u32 *queue)
{
	XCan *CanPtr;
	u8  *tempPtr;
	u8  i;

	//֡ͷ
	package->head = 0x55;

	//֡ͨ��
	CanPtr = InstancePtr;

	if(CanPtr->BaseAddress == XPAR_CAN_0_BASEADDR)//ȷ��ͨ��
	{
		package->CH =0x00;
	}else
	{
		package->CH = 0x01;
	}

	//֡ID
	package->id = (queue[0]&XCAN_IDR_ID1_MASK)>>XCAN_IDR_ID1_SHIFT;//�����ƶ�

	//���ݳ���
	package->dlc = (queue[1]&XCAN_DLCR_DLC_MASK)>>XCAN_DLCR_DLC_SHIFT;

	//������
	package->code = 0x00;

	//֡����
	tempPtr = (u8 *)&queue[2];


	//С��ת�����
	for(i = 0; i < 8; i++)
	{
		if(i < 4){
			//���λ�õ�ַ����
			package->data[i] = (u8)*(tempPtr+3-i);

		}else{
			package->data[i] = (u8)*(tempPtr+11-i);
		}

	}


	return 0;
}

// ���������ṹ������ת��Ϊ uint8_t �ֽ�����
// ����UDP�ֽڳ��ȣ�ָ��20��
//װ�ز������ұ�������ӵ���ߣ�����һ��ֱ�������������Ӧ�����������

void ChnPackage_convertToU8Array(ChnPackage *pack, uint8_t *outArray) {
    int index = 0;

    // ��ÿ���ֶΰ�˳������ֽ�������
    outArray[index++] = pack->head;
    outArray[index++] = pack->CH;

    // ���� 32-bit �� id
    outArray[index++] = (uint8_t)(pack->id >> 24);
    outArray[index++] = (uint8_t)(pack->id >> 16);
    outArray[index++] = (uint8_t)(pack->id >> 8);
    outArray[index++] = (uint8_t)(pack->id);

    // ���� 16-bit �� code
    outArray[index++] = (uint8_t)(pack->code >> 8);
    outArray[index++] = (uint8_t)(pack->code);

    // ���� dlc
    outArray[index++] = pack->dlc;  //outrArray[8];

    // ���� data ����  ˳�����
    memcpy(&outArray[index], pack->data, 8);

}

void CAN_Init(void)
{
	u32 filter;
	u8 status;

	//CAN ��ʼ��
	XCan_Initialize(&Can, 0);//��ָ��

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
//	//��һλΪ0������id 0x01�޷�����  0x00200000
//	//��ʶ��ַ���������ݴ洢λ��ӳ���ϵ�� ����һ���µ���֪���ᡣ
//	status = XCan_AcceptFilterSet(&Can, XCAN_AFR_UAF1_MASK, 0x00200000, 0x00200000); //mask  id
//	if(status != XST_SUCCESS)
//	{
//		xil_printf("filter err\n");
//	}
//
//	//�˲�������
//	XCan_AcceptFilterEnable(&Can, XCAN_AFR_UAF1_MASK);
//
//	//�ж��Ƿ���˲���
//	filter = XCan_AcceptFilterGetEnabled(&Can);
//	xil_printf("filter:%d\n", filter);


	XCan_EnterMode(&Can, XCAN_MODE_NORMAL);
	//CAN�жϳ�ʼ��

	//�ж�ʹ��XCAN_IXR_RXOK_MASK|XCAN_IXR_RXNEMP_MASK
	//XCan_InterruptEnable(&Can, XCAN_IXR_RXOK_MASK);
	XCan_InterruptDisable(&Can, XCAN_IXR_RXOK_MASK);
	XCan_InterruptDisable(&Can, XCAN_IXR_RXOFLW_MASK);


	XCan_InterruptEnable(&Can, XCAN_IXR_RXNEMP_MASK);//����Ӱ������ж����ǣ�����߼�����
	XCan_InterruptEnable(&Can, XCAN_IXR_TXOK_MASK);

}

//ͬ��Э�飬������λ������λ���Լ�CAN�ڵ��޷���Чͨ��
//���ݰ�������-���ݹ��ֽܷ�
//00-00-00-00--00-00-00-00
//ģʽ-ʱ��-֡����16λ  �ݶ�

void  CAN_CfgSendFrame(u32 *TxFrame, u8 *canData)
{
	u8 i;
	u32 *tempPtr = TxFrame;

	u8 *TxFramePtr;

	//����ID
	TxFrame[0] = XCan_CreateIdValue(canData[1], 0, 0, 0, 0);

	//���ݳ���
	TxFrame[1] = XCan_CreateDlcValue(8);//�������ݸ���������·��߼���������

	//ת����ַ��������
	TxFramePtr = (u8 *)(&tempPtr[2]);

	//����UDP����
	//udpRxCnt = canData[4]*10;//ע��qt�˿�˳����CAN��С�� ���˳��ͬ

	CANRxCounter_SetLimit(&canCounter, canData[5]<<8|canData[6]);//ע�������ϵ

	//�������,��С��ת�����Ǹ���Ȥ��̲���
	for(i = 0; i < 8; i++)
	{
		if(i > 3)
		{
			*TxFramePtr++ = i;//���ʹ���ڲ�ָ�룬��Ҫstatic��
		}else
		{
			//���﷽�����
			*TxFramePtr++ = canData[6 - i];//�߼���ϵ���󣬳���ָ������;
		}

	}


}

extern CAN_Buffer *queue;
#define TIME_THRESHOLD_MS 250

/*���ղ�����ƺ���*/
void CAN_PackUpload(void){

	static int round;
	static int reminder;
	static uint8_t arrayOrder;

	static u32 RxFrame[4];//�Ӷ�������ʱȡ������CAN֡

	//�жϽ��ն����Ƿ��

		//��ȡ��������
		CAN_Buffer_Dequeue(queue, RxFrame);

		//�����������
		ChnPackage_EnPackage(&Can, &package, RxFrame);//���pack����

		//@@@������ʾ��ӵ�package����
		//xil_printf("%d-%d-%d-%d-%d-%d-%d-%d\n", package.data[0],package.data[1],package.data[2],\
				package.data[3],package.data[4],package.data[5],package.data[6],package.data[7]);

		//Ԥ����Ϊ�ֽ����ݣ�װ��UDP�����У����з���׼��
		ChnPackage_convertToU8Array(&package, &UdpByteData[0]);//ת��Ϊ8�����ֽ�����	//ֱ��ʹ������������ȡ��ַ����

		//�����������������װ��ָ��
		//��ȡ���������ٿ���

		arrayOrder = CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt;//�������������

		//װ���ֽ����ݵ�UDP����
		prepareUdpdatagram(&udpdatagram[0], &UdpByteData[0], arrayOrder);

		//����UDP����
		CANRxCounter_Increment(&canCounter, 1);//������ƺ�������������������ַ���

		//������ʾ���ո����������ж�������ǿ����
		//xil_printf("UdpByteDataInCnt:%d\n", CANRxCounter_GetCount(&canCounter));

		//���մﵽ80��������
		//������ͣ���Ҫkeil-sdk�Լ�qt�㶼��Ҫ֪�����͸�����Ϣ
		//���udpdatagram��ʱֱ���ϴ�udp֡���ڴ泬ʱ����
		//����������ʲô��

		//timeoutģʽ�����ӣ�
		//��һ��CAN֡��ֱ�ӷ��ͣ����Կ��Լ����ⲿ��ʱ����ʱ��

		//��ȷif���������������߼������
		if((CANRxCounter_GetCount(&canCounter) == MAX_Udp_Rx_Cnt)){//|(elapsed_time_ms >= TIME_THRESHOLD_MS)){

			//����UDP��

			//xil_printf("transfer ok\n");

			//������һ�η��ͳ���timeout
			transfer_data(&udpdatagram[0], 17*CANRxCounter_GetCount(&canCounter));

			//���¼�ʱ��
			//last_received_time = get_time_ms();

			//����UDP��
			memset(udpdatagram, 0, sizeof(udpdatagram));

			//��֤���������ж�
			//xil_printf("send udp ok\n");ע��ȫ������������Ƚ�����

			//���ü���״̬
			CANRxCounter_Reset(&canCounter);//ʵʱ����CAN֡����

		}

#ifdef modTx
		//�жϷ���UDP��ʱ��
		if(CANRxCounter_GetLimit(&canCounter) < MAX_Udp_Rx_Cnt) //UDP���洢80��CAN��
		{//������CAN��С��UDP����С

			//@@@���ո�����Сʱ����֤��������
			//xil_printf("udpRxCnt:%d", udpRxCnt);//Ϊʲô�����޷����Ƶ������
			//xil_printf("UdpByteDataInCnt:%d \n", UdpByteDataInCnt);

			//�жϽ��ո�����Ԥ�ڸ�����ͬ
			if(CANRxCounter_GetCount(&canCounter) == CANRxCounter_GetLimit(&canCounter)){

				//����UDP��
				transfer_data(&udpdatagram[0], 17*CANRxCounter_GetCount(&canCounter));

				//����UDP��
				memset(udpdatagram, 0, sizeof(udpdatagram));

				//��֤���������ж�
				//xil_printf("send udp ok\n");ע��ȫ������������Ƚ�����

				//���ü���״̬
				CANRxCounter_Reset(&canCounter);//ʵʱ����CAN֡����

				command_code = 0;
			}

		}else
		{

			//�������㣬�����ж�����
			round = CANRxCounter_GetLimit(&canCounter)/MAX_Udp_Rx_Cnt; //��������ָ��UDP������Ҫ����
			reminder = CANRxCounter_GetLimit(&canCounter)%MAX_Udp_Rx_Cnt;//������������һ�ַ��͸���

			//������С��ָ����������װ������
			if(CANRxCounter_GetCount(&canCounter) <= round*MAX_Udp_Rx_Cnt){

				//ȡ�����㣺װ��������
				if(CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt == 0){

					//��������UDP��
					transfer_data(&udpdatagram[0], 80*17);

					//�������飺����ԭ��������Ⱦ
					memset(udpdatagram, 0, sizeof(udpdatagram));//��0��������

					//���ո���Ϊ400����ʱ���ѡ��������ȻҪ���ü���

					//��������
					//������������ξ�ȷ�жϴ���
					xil_printf("����can��������%d\n", CANRxCounter_GetCount(&canCounter));//�����������⣬Ϊ320��



				}else
				{
					//�����жϣ��ȴ�����
				}

				if((CANRxCounter_GetCount(&canCounter) == round*MAX_Udp_Rx_Cnt)&&reminder == 0x00){

					//�����߽����������ü���״̬
					CANRxCounter_Reset(&canCounter);//�����ļ������������ײ��������ж��߼�����
					command_code = 0;

				}

			}else{

				//����can������ͷ
				if(CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt ==reminder){

					//����ʣ��UDP��
					transfer_data(&udpdatagram[0], reminder*17);

					//��������Ϊ0
					memset(udpdatagram, 0, sizeof(udpdatagram));

					//���ü���״̬
					CANRxCounter_Reset(&canCounter);//�����ļ������������ײ��������ж��߼�����

					command_code = 0;
				}

			}

			}
#endif
}

/*ֱ��CAN֡�ϴ�UDP*/
//$$$queue-size���⣺Ϊʲô��ȫ��ͬ�߼���sizeȡֵ��ͬ
//$$$û�����Buffer���棬�������ݱȽ�����

void CAN_DirectUpload(void){

	static uint8_t arrayOrder;
	uint16_t cnt;
	u64 size;

	//ȡ������CAN������
	u32 RxFrame[4];

	//��ȡ�������� + 1000֡��ʱ�ж�
	CAN_Buffer_Dequeue(queue, RxFrame);


	//�����������
	ChnPackage_EnPackage(&Can, &package, RxFrame);

	//��ʱ��ӡ����
	//xil_printf("%x-%x-%x-%x-%x-%x-%x-%x\n", package.data[0],package.data[1],package.data[2],\
			package.data[3],package.data[4],package.data[5],package.data[6],package.data[7]);

	//Ԥ����Ϊ�ֽ����ݣ�װ��UDP�����У����з���׼��
	ChnPackage_convertToU8Array(&package, &UdpByteData[0]);//ת��Ϊ8�����ֽ�����	//ֱ��ʹ������������ȡ��ַ����

#ifdef modeTx
	//�����������������װ��ָ��
	arrayOrder = CANRxCounter_GetCount(&canCounter)%MAX_Udp_Rx_Cnt;//������������� Ϊʲô��85��

	//װ���ֽ����ݵ�UDP����
	prepareUdpdatagram(&udpdatagramMin[0], &UdpByteData[0], arrayOrder);
#endif

	//װ���ֽ����ݵ�UDP����
	prepareUdpdatagram(&udpdatagramMin[0], &UdpByteData[0], 0);

	//����UDP
	transfer_data(&udpdatagramMin[0], 17); //


//����ֱ��ECanTool���ͷ�ʽ
#ifdef modeTx
	//����udp���ͼ���
	CANRxCounter_IncrementUdp(&canCounter, 1);

	//��ӡUDP���ͼ�������
	cnt = CANRxCounter_GetUdpCount(&canCounter);
	xil_printf(" CANRxCounter_GetUdpCount:%x; limit:%x\n", cnt,CANRxCounter_GetLimit(&canCounter) );

	//��α�֤���޷���������ʱ��������������ƫ�����zynq��������ʵ��DDR֮��Ĺ�ϵ��
	//Ϊʲô��if�޷����룿
	if(CANRxCounter_GetUdpCount(&canCounter) == CANRxCounter_GetLimit(&canCounter)){

		command_code = 0;
		CANRxCounter_Reset(&canCounter);

		clear_CAN_Buffer(queue);
		xil_printf("udp send ok\n");
	}
#endif
}
