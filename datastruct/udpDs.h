/*
 * udpDs.h
 *
 *  Created on: 2024年9月24日
 *      Author: 77219
 */

#ifndef DATASTRUCT_UDPDS_H_
#define DATASTRUCT_UDPDS_H_

//udp
extern u8 udpBuf[2500];
extern u8 udpRxFlag; //注意接收重置操作
extern u16 udpCount;

u8 checkAndResetFlag(void );
void udpISR(void);

#endif /* DATASTRUCT_UDPDS_H_ */
