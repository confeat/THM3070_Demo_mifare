/********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: CCID.h
* Author: Yang Song
* Version: V1.0
* History:
*   2012-09-24 Original version
********************************************************************************/

#ifndef __CCID_H__
	#define __CCID_H__

#define CCID 1


//#define g_bUsbCcidRcvDataFlag	g_bUsbEp3RcvDataFlag
//#define	g_sUsbCcidTxLen			g_sUsbEp4TxLen		
//#define g_abUsbCcidRxBuf 		g_abUsbEpnBuf[3]
//#define g_abUsbCcidTxBuf		g_abUsbEpnBuf[4]
extern u8 bCardInserted;
extern u8 g_bUsbCcidRcvDataFlag;	
extern u16	g_sUsbCcidTxLen;
extern u8 g_sUsbCcidTxTimes;			
extern u8 g_abUsbCcidRxBuf[]; 		
extern u8 g_abUsbCcidTxBuf[];
extern u8 slotchange;
extern u8  g_sUsbCcidTxnPakageFlag;;	
extern __align(4) uint8_t	IOBuf[256];	

#define g_abUsbCcidCmdHeader	g_abUsbCcidTxBuf


extern const u8 ccidInterfaceDescriptor[];
extern const u8 ccidSmartCardDeviceClassDescriptor[];
extern const u8 ccidEndpointDescriptor[];
extern const u8 ccidReaderStringDescriptor[];

//	Macros for indicating ICC status in the CCID
#define	ICCPWRON								0xA5
#define	ICCPWROFF								0x5A

//	CCID Class Specific Request
#define	ABORT									0x01
#define	GET_CLOCK_FREQUENCIES					0x02
#define	GET_DATA_RATES							0x03

//	Command Pipe,Bulk-OUT Messages
#define	PC_TO_RDR_ICC_POWER_ON					0x62
#define	PC_TO_RDR_ICC_POWER_OFF					0x63
#define	PC_TO_RDR_GET_SLOT_STATUS				0x65
#define	PC_TO_RDR_XFR_BLOCK						0x6F
#define	PC_TO_RDR_GET_PARAMETERS				0x6C
#define	PC_TO_RDR_RESET_PARAMETERS				0x6D
#define	PC_TO_RDR_SET_PARAMETERS				0x61
#define	PC_TO_RDR_ESCAPE						0x6B
#define	PC_TO_RDR_ICC_CLOCK						0x6E
#define	PC_TO_RDR_T0_APDU						0x6A
#define	PC_TO_RDR_SECURE						0x69
#define	PC_TO_RDR_MECHANICAL					0x71
#define	PC_TO_RDR_ABORT							0x72
#define	PC_TO_RDR_SET_DATE_RATE_AND_CLOCK_FREQ	0x73

//	Response Pipe,Bulk-IN Messages
#define	RDR_TO_PC_DATA_BLOCK					0x80
#define	RDR_TO_PC_SLOT_STATUS					0x81
#define	RDR_TO_PC_PARAMETERS					0x82
#define	RDR_TO_PC_ESCAPE						0x83
#define	RDR_TO_PC_DATA_RATE_AND_CLOCK_FREQ		0x84

//	bError definition
#define	CMD_ABORTED								0xFF

//	Length of header
#define	CCID_HEADER_LENGTH						0x0A

#define	CCID_MESSAGE_TYPE_ADDR					0	//	bMessageType
#define	CCID_DWLENGTH_ADDR						1	//	st_ccidCmdMessage.dwLength
#define	CCID_SLOT_ADDR							5	//	bSlot
#define	CCID_SEQ_ADDR							6	//	bSeq

//	Variables and functions defined in CCID
typedef struct{
u32 dwLength;
u8	bMessageType;
u8	bSlot;
u8	bSeq;
u8	bLockCard;
u8	bIccPowerOn;
u8	bWaitForAbort;									//	Flag indicates ABORT request has been received,but PC_to_RDR_Abort is not received
u8	readerid;
}CCID_COMMAND_MESSAGE;

extern CCID_COMMAND_MESSAGE st_ccidCmdMessage;

typedef	u16 (* CCID_CMD_FUNC) (CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);

extern u16	ccidPCtoRDRIccPowerOn(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRIccPowerOff(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRGetSlotStatus(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRXfrBlock(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRGetParameters(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRResetParameters(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRSetParameters(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDREscape(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRIccClock(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRT0APDU(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRSecure(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRMechanical(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRAbort(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);
extern u16	ccidPCtoRDRSetDataRateAndClockFrequency(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * inputbuf,u8 * outputbuf);

extern const u8 ccidMessageType[];
extern const CCID_CMD_FUNC	ccidPCtoRDRHandle[];

extern u16	ccidWaitForAbort(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * outputbuf);
extern u16	ccidSendProcedureByte(u8 slot, u8 seq, u8 pb, u8 * outputbuf);
extern u16	ccidRDRtoPCTimeOutHandle(u8 messagetype,u8 slot,u8 seq, u8 * outputbuf);
extern u16	ccidRDRtoPCSlotDoesNotExistHandle(u8 messagetype,u8 slot,u8 seq,u8 field9, u8 * outputbuf );

extern void	ccidRequest(u8 req);
extern void	CCID_main(void);
extern void	ccidDataSeparate(CCID_COMMAND_MESSAGE *st_cmdmessage,u8 * ccidheaderbuf);
extern u16	ccidCmdHandler(CCID_COMMAND_MESSAGE *st_cmdmessage, u8* inputbuf, u8* outputbuf);

extern u8	ccidPowerOnProcess(u8 readerid, u8 PowerSelect, u8 * atrbuf);
extern u8	ccidPowerOffProcess(u8 readerid);
extern u8	ccidSetParametersProcess(u8 readerid,u8 protocol, u8 * ProtocolDataStructure, u8 len);
extern u8	ccidDataRateAndClockFrequencyProcess(u8 readerid,u8 *clkfreq_datarate);
extern u16	ccidCommandProcess(u8 readerid, u8 * cmddatabuf, u16 len);
extern u8 __FindCard(void);
extern u8 __CardAbsent( void );

extern u8	ICCPowerOn;
extern u8   mode_flag;
extern u8  apdusendtime;



#endif //__CCID_H__
