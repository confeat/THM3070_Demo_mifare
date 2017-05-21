		  /********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: ISO14443.h
* Author: Cui Lu
* Version: V1.0
* History:
*   2013-05-30 Original version
********************************************************************************/

#ifndef __ISO14443_H__
	#define __ISO14443_H__

#include "usb_lib.h"
#include "usb_bot.h"
#include "usb_istr.h" 

#define MAXSLOT 0x01

//represents the cards that PCD	holds, after deselect, card is released.
struct ST_PICC{		
	//u8 UID[10];
	u8 TYPE;
	u8 CID;
	u8 DSDR; //values to be 00,01,10,11
	u8 FSCI;//big4-1 represents FSCI	
	u8 FWI;//default 4
	u8 SFGI;//default 0	
	u8 WTXM;//the index multiplier of WTX
	u8 blocknumber;
	u8 withCID;//set if transmision includes CID
	u8 MBLI;
	u8 minTR2;//b3 and b2 represents the minimum TR2
};

extern struct ST_PICC mypicc[MAXSLOT];

extern u8  HandleCollision;

 // Error return values 
#define ERR_TIMEOUT_WUPA         		-1
#define ERR_REV_WUPA         			-2
#define ERR_TIMEOUT_WUPB         		-3//FD
#define ERR_REV_WUPB         			-4
#define ERR_REV_AC    					-5   
#define ERR_REV_ATS        				-6 
#define ERR_REV_ATTRIB_RESPONSE			-7 
#define ERR_TIMEOUT_REV_ATTRIB			-8
#define ERR_FE_REV_ATTRIB				-9

#define ERR_RECEIVE_SYNTAX              -10 //F6
#define ERR_RECEIVE_CRC      			-11//F5
#define ERR_RECEIVE_TIMEOVER     		-12
#define ERR_RECEIVE_DATOVER     		-13
#define ERR_RECEIVE_FERR        		-14
#define ERR_RECEIVE_PERR        		-15
#define ERR_RECEIVE_CERR        		-16
#define ERR_RECEIVE_OTHER        		-17	//EF



#define MAXSLOT 0x01

#define MAX_RETRANS_APDU 2	//max retransmission time during APDU transmission
#define MAX_RETRANS_WUPAB 2	//max retransmission time of WUPA/WUPB if timeout occurred during anticollision
#define MAX_NAK 3
// If set 1, the funciton "sendAC" will send anticollision commands
//If set 0, when detect collision, the function will simply return error code
//#define HandleCollision 0 // 
#define DFWI 4// default FWI
#define FSDI 0x08 //b4-b1 defines max receive frame size of PCD, e.g. 0x08
#define DSIDRI 0x00 // b8-b5 DSI; B4-B1 DRI;
#define TR0TR1 0x00  //b8-b7 TR0; b6-b5 TR1;
#define EOFSOF 0x00  // b4 EoF; B3 SOF;
//#define EXTENDATQB 0x00//bit 5 set if extended ATQB supported //not finished

#define MAXSLOT 0x01



extern void THM_En_2Tx(void);//open RF
extern void THM_DIs_2Tx(void);//close RF
extern void ISO14443_carrier_on(void);//open RF,set WWT ,initiate buffer
extern void ISO14443_carrier_off(void);//close RF;open RF,set WWT ,initiate buffer
extern void ISO14443_carrier(u8 Type);//close RF;open RF,set WWT ,initiate buffer

extern void	ISO14443_reset(u8 t_reset);
extern void	ISO14443_powerdown(void);  
extern s16	ISO14443_polling(u8 cid, u8 *rbuffer); //activate card for either typeA/Type B ;return error code

//extern s16	ISO14443_PPS(u8 cid,u8 fidi);//return error code | 0
extern s16	ISO14443_sendAPDU(u8 cid,u8 *tbuffer,u16 length,u8 *rbuffer);//main apdu handle 
extern s16	ISO14443_deSelect(u8 cid,u8 *rbuffer);//cID released
extern s16	ISO14443_presentCheck(u8 cid,u8 mode);

//extern s16	ISO14443_REQA(u8 *ATQA);  //initiate sequence number
extern s16	ISO14443_WUPA(u8 *ATQA);
extern s16	ISO14443_Anticollision_TypeA(u8 *rbuffer);	//anti-collision 
extern s16	ISO14443_RATS(u8 cid,u8 *ATS); 
extern void	ISO14443_HLTA(void);

//Type B
extern u8	ISO14443_REQB(u8 *ATQB);
extern s16	ISO14443_WUPB(u8 nslots,u8 *ATQB);
extern s16	ISO14443_Attrib(u8 *ATQB,u8 mycid,u8 *rbuffer);
extern s16	ISO14443_SlotMarker(u8 slotnum,u8 *rbuffer);
extern s16	ISO14443_Anticollision_TypeB(u8 cid,u8 *rbuffer);	//anti-collision , CID assigned
extern void	ISO14443_HLTB(u8 *PUPI);
extern s16	ISO14443_PARAMB(u8 fidi);	 
extern void delay_ms(unsigned int ms);
extern s16 ISO14443_PPS(u8  Type,u8 Send_Rate,u8 Rec_Rate);

//static functions 
static s16  sendAC(u8 caslevel, u8 *Iddata);
static void initPICC (u8 mycid);
static void deletePICC (u8 mycid); 
static void	sendBlock(u8 PCB,u8 CID,u8 *tbuffer,u16 length);
static s16  recvBlock(u8 *PCB,u8 *CID,u16 *rLEN,u8 *rbuffer);  

#endif //__IS014443_H__

