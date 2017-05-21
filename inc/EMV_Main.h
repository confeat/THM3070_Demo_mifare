							  /********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: ISO14443.h
* Author: Cui Lu
* Version: V1.0
* History:
*   2013-05-30 Original version
********************************************************************************/

#ifndef __EMV_Main_H__
	#define __EMV_Main_H__

#define ERR_Collision       			-30

#define tp 6//use to adjust tp time for WUPA and WUPB
#define fdtpcd 1//lichong
#define Treset 0x8	//adjust RF reset time 
#define tpause 0x8
#define Maxpolling 10

extern u8 p_TYPE_A;
extern u8 p_TYPE_B;
//EMV main loop functions
extern s16	EMV_preValidate (u8 *rbuffer);
extern void	EMV_loopback (void);//

//EMV procedure functions
extern void	EMV_polling(void); //
extern void	EMV_polling1(void); //
extern s16	EMV_Collision_Activate(u8 CID, u8 *rbuffer);
extern void	EMV_removal(void); //
extern s16	EMV_RATS(u8 CID, u8 *rbuffer);
extern s16	EMV_Attrib(u8 CID, u8 *rbuffer);
extern u8   EMV_loopback_Analog(unsigned char picctype);
extern s16	EMV_Collision_Activate_CID(u8 CID,u8 *rbuffer);
extern s16	ISO14443_Anticollision_TypeA_Analog(u8 *rbuffer);
extern s16	ISO14443_Anticollision_TypeB_Analog(u8 cid,u8 *rbuffer);

#endif //__EMV_Main__
