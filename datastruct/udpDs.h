/*
 * udpDs.h
 *
 *  Created on: 2024��9��24��
 *      Author: 77219
 */

#ifndef DATASTRUCT_UDPDS_H_
#define DATASTRUCT_UDPDS_H_

//udp
extern u8 udpBuf[2500];
extern u8 udpRxFlag; //ע��������ò���
extern u16 udpCount;

u8 checkAndResetFlag(void );
void udpISR(void);

#endif /* DATASTRUCT_UDPDS_H_ */
