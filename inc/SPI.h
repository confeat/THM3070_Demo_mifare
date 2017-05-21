/********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: SPI.h
* Author: Yang Song
* Version: V1.0
* History:
*   2012-09-24 Original version
********************************************************************************/

#ifndef __SPI_H__
#define __SPI_H__

extern void SPI_SendBuff(unsigned char *buf,unsigned int num);
extern void SPI_RecvBuff(unsigned char *buf,unsigned int num);

extern void SPI_FRAME_START(void);  
extern void SPI_FRAME_END(void); 

extern void _send_byte(unsigned char dat);

extern void SPI_Init_GPIO(void);

extern void LED_Off(void);
extern void LED_On(void);
extern void Buzzer_On(void);
extern void Buzzer_Off(void);

#endif //__SPI_H__

