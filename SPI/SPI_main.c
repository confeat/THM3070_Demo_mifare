/********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: SPI
* Author: Yang Song
* Version: V1.0
* History:
*   2012-09-24 Original version
********************************************************************************/

#include "thm3060.h" 
#include "usb_lib.h"
#include "usb_bot.h"
#include "usb_istr.h"

BYTE dummy_bytes[16] = {0x11,0x22,0x33,0x44,0};

BYTE	SPI_RcvAPDU(void)
{
	SPI_RxNByte(g_abComBuf	,5); // receive cmd header

	if(((INS == WRFLASH) || (INS == VERIFY) || (INS == ERFLASH) )&& (P3!= 0))
	{	
		SPI_RxNByte(IOBuf,P3);
	}
	if(CLA != 0 )
		return 0;
	return 1;	
}

//====================================================
//	Tx response data
void	SPI_Response(void)
{
	if(((INS == GETRDMN)||(INS == GETRESPONSE)||(INS == RDFLASH)) && ((SWptr == SUCCESS)||(SWptr == LEFTDAT)))
	{	
		if(P3 == 0x00)
			SPI_TxNByte(IOBuf,0x100);
		else
			SPI_TxNByte(IOBuf,P3);
	}
}

/***********************************************************************
//	Send status word
***********************************************************************/
void	SPI_TxStatus(int ptr)
{
	BYTE tempSW[2];

	tempSW[0] = SW[ptr];
	tempSW[1] = SW[ptr+1];

	if(ptr != LEFTDAT)
	{
		SPI_TxNByte(tempSW,2);
	}
	else		   //	61XX
	{
	
	}
}



