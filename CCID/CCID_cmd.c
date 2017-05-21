 /********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: CCID
* Author: Yang Song
* Version: V1.0
* History:
*   2012-09-24 Original version
********************************************************************************/
#include "usb_lib.h"
#include "string.h"
#include "ccid.h" 
#include "thm3070.h" 
#include "ISO14443.h"
#include "ISO15693.h"
#include "EMV_Main.h" 
#include "Crypto1.h"
 
u8  apdusendtime =0;


/*******************************************************************************
* Function    : ccidCommandProcess
* Description : polling process, set Flag p_TYPE_A or p_TYPE_B to 1 if detect any TypeA or TypeB card.
* Arguments   : cmddatabuf --- IN:first 5 bytes is cmd, others is data	 OUT:data for response
				len   --- the total length of cmd and data
* Return      :	0     ---  time out or failed
				other ---  the length of data for response
*******************************************************************************/
u16	ccidCommandProcess(u8 readerid, u8 * cmddatabuf, u16 len)
{
	u16 iLen;
	s16 rvsta=0;
    u32 fwiwtxm = 0;
	u8 mlen;
	u8 i;   
	u8 mUID[4];
	u8 afi;
	

	if(cmddatabuf[0] == 0xFF)
	{
		mode_flag = 2;
		if(apdusendtime == 0)
		{
			ISO14443_reset(Treset+2);
			apdusendtime = 1;
			delay_ms(1);
		}
		switch(cmddatabuf[1])
		{
			case	0x00:	ISO14443_carrier(cmddatabuf[5]);							 //ff 00 00 00 01 xx  
							cmddatabuf[0] = 0x90;										 // xx:00 关场
							cmddatabuf[1] = 0x00;										 //	  :01 开场TYPEA
							return 2;													 //	  :02 开场TYPEB

			case	0x01:	ISO14443_reset(Treset+2);									 //ff 01 00 00 00 	对卡片进行掉电复位
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case	0x02:	EMV_polling();												 //ff 02 00 00 00 	 polling
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case    0x03:	SleepMode(cmddatabuf[5]);									 //ff 03 00 00 01 xx 
							cmddatabuf[0] = 0x90;										 // xx:00 休眠
							cmddatabuf[1] = 0x00;										 //	  :01 退出休眠
							return 2;

			case	0x04:	rvsta=ISO14443_WUPA(cmddatabuf);							 //ff 04 00 00 00	WUPA
							if(rvsta>0) 												 
							{
								cmddatabuf[rvsta] = 0x90;	
								cmddatabuf[rvsta+1] = 0x00;
								return (rvsta+2);
							}
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
			case	0x05:	rvsta=ISO14443_WUPB(0x00,cmddatabuf);						 //ff 05 00 00 00	WUPB		
				 			if(rvsta>0) 
							{
								cmddatabuf[rvsta] = 0x90;	
								cmddatabuf[rvsta+1] = 0x00;
								return (rvsta+2);
							}
							else{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case	0x06:	rvsta=EMV_Collision_Activate_CID(0x00,cmddatabuf);			  //ff 06 00 00 00	防冲突激活卡
							if(rvsta>0) 
							{
								cmddatabuf[rvsta] = 0x90;	
								cmddatabuf[rvsta+1] = 0x00;
								return (rvsta+2);
							}
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case	0x07:	rvsta=EMV_RATS(0x00,cmddatabuf);							  //ff 07 00 00 00	
							if(rvsta>0) 
							{
								cmddatabuf[rvsta] = 0x90;	
								cmddatabuf[rvsta+1] = 0x00;
								return (rvsta+2);
							}
							else{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
 			case	0x08:	rvsta=EMV_Attrib(0x00,cmddatabuf);							  //ff 08 00 00 00
							if(rvsta>0) 
							{
								cmddatabuf[rvsta] = 0x90;	
								cmddatabuf[rvsta+1] = 0x00;
								return (rvsta+2);
							}
							else{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
																 
			case	0x09:	THM_SendFrame(cmddatabuf+5,cmddatabuf[4]);					   //ff 09 00 00 xx1 xx2
							rvsta = THM_WaitReadFrame(&iLen, cmddatabuf);				   //xx1:需发送数据的长度
							if(iLen>0) 													   //xx2:需发送数据的内容
							{
								cmddatabuf[iLen] = 0x90;	
								cmddatabuf[iLen+1] = 0x00;
								return (iLen+2);
							}
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case	0x0a:	EMV_loopback();													//ff 0a 00 00 00
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case	0x0b:	EMV_loopback_Analog(cmddatabuf[5]);								//ff 0b 00 00 01 xx
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;
			case    0x0C:   rvsta=ISO14443_sendAPDU(0x00,cmddatabuf+5,len,cmddatabuf);		//ff 0c 00 00 xx1 xx2
							if(rvsta>0) 													//xx1:需发送数据的长度
							{																//xx2:需发送数据的内容
								cmddatabuf[rvsta] = 0x90;	
								cmddatabuf[rvsta+1] = 0x00;
								return (rvsta+2);
							}
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case	0x0D:	rvsta=ISO14443_PPS(cmddatabuf[5],cmddatabuf[6],cmddatabuf[7]);	 //ff 0d 00 00 03 xx xx xx
							if(rvsta>0) 
							{
								cmddatabuf[0] = 0x90;	
								cmddatabuf[1] = 0x00;
								return 2;
							}
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case	0x0e:	cmddatabuf[0] = THM_ReadReg(cmddatabuf[5]);						  //ff 0e 00 00 01 xx  读寄存器，xx为寄存器地址
							cmddatabuf[1] = 0x90;	
							cmddatabuf[2] = 0x00;
							return 3;
														
			case	0x0f:	THM_WriteReg(cmddatabuf[5],cmddatabuf[6]);						  //ff 0f 00 00 00 xx1 xx2  写寄存器，xx1为寄存器地址，xx2为寄存器的值
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case    0x11:	fwiwtxm = ((cmddatabuf[5]<<16)&&(cmddatabuf[6]<<8)&&(cmddatabuf[7]));	 //ff 11 00 00 00 xx1 xx2 xx3 设置帧等待时间
							THM_SetFWT(fwiwtxm);
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;
									
			case    0x82:	THM_DIs_2Tx;													   //ff 82 00 xx1 06 xx xx xx xx xx xx
							delay_ms(2);													   //M1使用，进行寻卡操作，并加载秘钥
							THM_En_2Tx();													   //xx1:60表示装载A秘钥，61表示装载B秘钥
							delay_ms(2);													   //xx:表示六字节秘钥
							rvsta = THM_MFindCard( mUID );
							delay_ms(2);
							THM_MInitialKEY( cmddatabuf[3], cmddatabuf+5 );
							if( 0!= rvsta)
							{
							    cmddatabuf[0] = 0x65; // SW1
								cmddatabuf[1] = rvsta; // SW2 
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}					
			case    0x86:	rvsta = THM_MAuthentication( cmddatabuf[8], cmddatabuf[7] );		 //ff 86 00 00 05 01 00 xx1 xx2 00	  
							if( 0!= rvsta)														 //M1使用，认证操作
							{																	 //xx1:需认证块号
							    cmddatabuf[0] = 0x65; // SW1									 //xx2:60 使用A秘钥进行认证。61 使用B秘钥进行认证
								cmddatabuf[1] = rvsta; // SW2 
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}
			case    0x87:	rvsta = THM_MAuthenticationT( cmddatabuf[8], cmddatabuf[7] );		  //ff 86 00 00 05 01 00 xx1 xx2 00
							if( 0!= rvsta)														  //M1使用，对同一卡片二次认证使用
							{																	  //xx1:需认证块号
							    cmddatabuf[0] = 0x65; // SW1									  //xx2:60 使用A秘钥进行认证。61 使用B秘钥进行认证
								cmddatabuf[1] = rvsta; // SW2 
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}
			case    0xb0:	rvsta = THM_MRead(cmddatabuf[3],&mlen, cmddatabuf);					   //ff b0 00 xx1 xx2 
							if( 0!= rvsta)														   //M1使用，读操作
							{																	   //xx1为读取块号
							    cmddatabuf[0] = 0x65; // SW1									   //xx2为读取数据长度
								cmddatabuf[1] = rvsta; // SW2 
								return 2;						
							}	
							else
							{				
								cmddatabuf[mlen] = 0x90; // SW1
								cmddatabuf[mlen+1] = 0x00; // SW2 
								return mlen+2;		
							}
			case    0xd6:	rvsta = THM_MWrite(cmddatabuf[3], cmddatabuf+5 );						//ff d6 00 xx1 xx2 xx 
							if( 0!= rvsta)															//M1使用，写操作
							{																		//xx1:块号
							    cmddatabuf[0] = 0x65; // SW1										//xx2:要写入数据的长度
								cmddatabuf[1] = rvsta; // SW2 										//xx :要写入的数据内容
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}

			case    0xf0:	rvsta = THM_Mchange(cmddatabuf[5], cmddatabuf[6],cmddatabuf+7);			 //ff f0 00 04 06 xx1 xx2 xx xx xx xx
							if( 0!= rvsta)															 //M1使用，加减值操作
							{																		 //xx1:c1表示加值操作，c0表示减值操作
							    cmddatabuf[0] = 0x65; // SW1										 //xx2:块号
								cmddatabuf[1] = rvsta; // SW2 										 //xx :需要加减的值
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}

			case    0x93:   																		  //ISO15693相关操作
			switch(cmddatabuf[5] ) 
			{case  0x01:  	iLen=ISO15693_ReadCARD(cmddatabuf);										  //ff 93 00 00 01 01
							if(iLen == 0x08)														  //15693协议使用，清点命令
							{						
								cmddatabuf[iLen] = 0x90;	
								cmddatabuf[iLen+1] = 0x00;
								return (iLen+2);
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
			case  0x02:  	iLen=ISO15693_Stayquiet();												  //ff 93 00 00 01 02
							if(iLen == 1)															  //15693协议使用，静默命令
							{						
								cmddatabuf[0] = 0x90;	
								cmddatabuf[1] = 0x00;
								return 2;
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
			case  0x20: 	iLen=ISO15693_ReadBlock(cmddatabuf[6],cmddatabuf);						   //ff 93 00 00 02 20 xx
							if(iLen>0)																   //15693协议使用，读块命令
							{																		   //xx:需要读取的块号
									cmddatabuf[iLen] = 0x90;	
									cmddatabuf[iLen+1] = 0x00;
									return (iLen+2);
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}			 	
			case  0x21: 	iLen=ISO15693_WriteBlock(cmddatabuf[6],cmddatabuf[7],cmddatabuf+8);			//ff 93 00 00 07 21 xx1 xx2 xx xx xx xx
							if(iLen==1)																	//15693协议使用，写块命令
							{																			//xx1:块号
									cmddatabuf[0] = 0x90;												//xx2:数据长度
									cmddatabuf[1] = 0x00;												//xx :数据内容
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}	
			case  0x22: 	iLen=ISO15693_LockBlock(cmddatabuf[6]);										 //ff 93 00 00 02 22 xx
							if(iLen==1)																	 //15693协议使用，锁块命令
							{																			 //xx:块号
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}
			case  0x23: 	iLen=ISO15693_ReadMBlock(cmddatabuf[6],cmddatabuf[7],cmddatabuf);			  //ff 93 00 00 02 23 xx1 xx2
							if(iLen>0)																	  //15693协议使用，读多块命令
							{																			  //xx1:块号
									cmddatabuf[iLen] = 0x90;											  //xx2:块长度
									cmddatabuf[iLen+1] = 0x00;
									return (iLen+2);
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}			 	
			case  0x24: 	iLen=ISO15693_WriteMBlock(cmddatabuf[6],cmddatabuf[7],cmddatabuf[8],cmddatabuf+9);	 //ff 93 00 00 04 24 xx1 xx2 xx3
							if(iLen==1)																			 //15693协议使用，写多块命令
							{																					 //xx1:开始块号
									cmddatabuf[0] = 0x90;														 //xx2:块数量
									cmddatabuf[1] = 0x00;														 //xx3:数据长度
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}
			case  0x25: 	iLen=ISO15693_Select();																  //ff 93 00 00 01 25
							if(iLen==1)																			  //15693协议使用，选卡命令
							{						
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
			case  0x26: 	iLen=ISO15693_Reset2ready();														   //ff 93 00 00 01 26
							if(iLen==1)																			   //15693协议使用，卡片重置为就绪状态命令
							{						
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
			case  0x27: 	iLen=ISO15693_WriteAFI(cmddatabuf[6]);													//ff 93 00 00 02 27 xx
							if(iLen==1)																				//15693协议使用，写AFI命令
							{																						//xx:AFI值
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}
			case  0x28: 	iLen=ISO15693_LockAFI();																 //ff 93 00 00 01 28
							if(iLen==1)																				 //15693协议使用，锁定AFI命令
							{						
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
						
			case  0x29: 	iLen=ISO15693_WriteDSFID(cmddatabuf[6]);												  //ff 93 00 00 02 29 xx
							if(iLen==1)																				  //15693协议使用，写DSFID命令
							{																						  //xx:DSFID值
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}
			case  0x2a: 	iLen=ISO15693_LockDSFID();																   //ff 93 00 00 01 2A
							if(iLen==1)																				   //15693协议使用，锁DSFID命令
							{																						   
									cmddatabuf[0] = 0x90;	
									cmddatabuf[1] = 0x00;
									return 2;
							}  
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case  0x2B: 	iLen=ISO15693_GetSysInf(cmddatabuf);														//ff 93 00 00 01 2B
							if(iLen>0)																					//15693协议使用，获得系统信息命令
							{						
									cmddatabuf[iLen] = 0x90;	
									cmddatabuf[iLen+1] = 0x00;
									return (iLen+2);
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}

			case  0x2C: 	iLen=ISO15693_ReadMBlockStatus(cmddatabuf[6],cmddatabuf[7],cmddatabuf);	
							if(iLen>0)
							{						
									cmddatabuf[iLen] = 0x90;	
									cmddatabuf[iLen+1] = 0x00;
									return (iLen+2);
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}
										
			case  0x2d: 	iLen = ISO15693_ReadCARD_WithAFI(cmddatabuf,cmddatabuf[6]);									  //ff 93 00 00 01 2d xx
							if(iLen == 0x08)																			  //15693协议使用，带AFI值的清点操作
							{																							  //xx:AFI的值
								cmddatabuf[iLen] = 0x90;	
								cmddatabuf[iLen+1] = 0x00;
								return (iLen+2);
							} 
							else
							{
								cmddatabuf[0] = 0x6D;
								cmddatabuf[1] = 0x00;
								return 2;
							}															
		}					
		default:		return 0;
		}		
	} 
	else
	{
		rvsta=ISO14443_sendAPDU(0x00,cmddatabuf,len,cmddatabuf);
		if(rvsta>=0)
			return (rvsta);
		else
			return 0;	
	}

}
