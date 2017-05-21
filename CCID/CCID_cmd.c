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
							cmddatabuf[0] = 0x90;										 // xx:00 �س�
							cmddatabuf[1] = 0x00;										 //	  :01 ����TYPEA
							return 2;													 //	  :02 ����TYPEB

			case	0x01:	ISO14443_reset(Treset+2);									 //ff 01 00 00 00 	�Կ�Ƭ���е��縴λ
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case	0x02:	EMV_polling();												 //ff 02 00 00 00 	 polling
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case    0x03:	SleepMode(cmddatabuf[5]);									 //ff 03 00 00 01 xx 
							cmddatabuf[0] = 0x90;										 // xx:00 ����
							cmddatabuf[1] = 0x00;										 //	  :01 �˳�����
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

			case	0x06:	rvsta=EMV_Collision_Activate_CID(0x00,cmddatabuf);			  //ff 06 00 00 00	����ͻ���
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
							rvsta = THM_WaitReadFrame(&iLen, cmddatabuf);				   //xx1:�跢�����ݵĳ���
							if(iLen>0) 													   //xx2:�跢�����ݵ�����
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
							if(rvsta>0) 													//xx1:�跢�����ݵĳ���
							{																//xx2:�跢�����ݵ�����
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

			case	0x0e:	cmddatabuf[0] = THM_ReadReg(cmddatabuf[5]);						  //ff 0e 00 00 01 xx  ���Ĵ�����xxΪ�Ĵ�����ַ
							cmddatabuf[1] = 0x90;	
							cmddatabuf[2] = 0x00;
							return 3;
														
			case	0x0f:	THM_WriteReg(cmddatabuf[5],cmddatabuf[6]);						  //ff 0f 00 00 00 xx1 xx2  д�Ĵ�����xx1Ϊ�Ĵ�����ַ��xx2Ϊ�Ĵ�����ֵ
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;

			case    0x11:	fwiwtxm = ((cmddatabuf[5]<<16)&&(cmddatabuf[6]<<8)&&(cmddatabuf[7]));	 //ff 11 00 00 00 xx1 xx2 xx3 ����֡�ȴ�ʱ��
							THM_SetFWT(fwiwtxm);
							cmddatabuf[0] = 0x90;
							cmddatabuf[1] = 0x00;
							return 2;
									
			case    0x82:	THM_DIs_2Tx;													   //ff 82 00 xx1 06 xx xx xx xx xx xx
							delay_ms(2);													   //M1ʹ�ã�����Ѱ����������������Կ
							THM_En_2Tx();													   //xx1:60��ʾװ��A��Կ��61��ʾװ��B��Կ
							delay_ms(2);													   //xx:��ʾ���ֽ���Կ
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
							if( 0!= rvsta)														 //M1ʹ�ã���֤����
							{																	 //xx1:����֤���
							    cmddatabuf[0] = 0x65; // SW1									 //xx2:60 ʹ��A��Կ������֤��61 ʹ��B��Կ������֤
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
							if( 0!= rvsta)														  //M1ʹ�ã���ͬһ��Ƭ������֤ʹ��
							{																	  //xx1:����֤���
							    cmddatabuf[0] = 0x65; // SW1									  //xx2:60 ʹ��A��Կ������֤��61 ʹ��B��Կ������֤
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
							if( 0!= rvsta)														   //M1ʹ�ã�������
							{																	   //xx1Ϊ��ȡ���
							    cmddatabuf[0] = 0x65; // SW1									   //xx2Ϊ��ȡ���ݳ���
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
							if( 0!= rvsta)															//M1ʹ�ã�д����
							{																		//xx1:���
							    cmddatabuf[0] = 0x65; // SW1										//xx2:Ҫд�����ݵĳ���
								cmddatabuf[1] = rvsta; // SW2 										//xx :Ҫд�����������
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}

			case    0xf0:	rvsta = THM_Mchange(cmddatabuf[5], cmddatabuf[6],cmddatabuf+7);			 //ff f0 00 04 06 xx1 xx2 xx xx xx xx
							if( 0!= rvsta)															 //M1ʹ�ã��Ӽ�ֵ����
							{																		 //xx1:c1��ʾ��ֵ������c0��ʾ��ֵ����
							    cmddatabuf[0] = 0x65; // SW1										 //xx2:���
								cmddatabuf[1] = rvsta; // SW2 										 //xx :��Ҫ�Ӽ���ֵ
								return 2;						
							}	
							else
							{
								cmddatabuf[0] = 0x90; // SW1
								cmddatabuf[1] = 0x00; // SW2 
								return 2;		
							}

			case    0x93:   																		  //ISO15693��ز���
			switch(cmddatabuf[5] ) 
			{case  0x01:  	iLen=ISO15693_ReadCARD(cmddatabuf);										  //ff 93 00 00 01 01
							if(iLen == 0x08)														  //15693Э��ʹ�ã��������
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
							if(iLen == 1)															  //15693Э��ʹ�ã���Ĭ����
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
							if(iLen>0)																   //15693Э��ʹ�ã���������
							{																		   //xx:��Ҫ��ȡ�Ŀ��
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
							if(iLen==1)																	//15693Э��ʹ�ã�д������
							{																			//xx1:���
									cmddatabuf[0] = 0x90;												//xx2:���ݳ���
									cmddatabuf[1] = 0x00;												//xx :��������
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}	
			case  0x22: 	iLen=ISO15693_LockBlock(cmddatabuf[6]);										 //ff 93 00 00 02 22 xx
							if(iLen==1)																	 //15693Э��ʹ�ã���������
							{																			 //xx:���
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
							if(iLen>0)																	  //15693Э��ʹ�ã����������
							{																			  //xx1:���
									cmddatabuf[iLen] = 0x90;											  //xx2:�鳤��
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
							if(iLen==1)																			 //15693Э��ʹ�ã�д�������
							{																					 //xx1:��ʼ���
									cmddatabuf[0] = 0x90;														 //xx2:������
									cmddatabuf[1] = 0x00;														 //xx3:���ݳ���
									return 2;
							} 
							else
							{
									cmddatabuf[0] = 0x6D;
									cmddatabuf[1] = 0x00;
									return 2;
							}
			case  0x25: 	iLen=ISO15693_Select();																  //ff 93 00 00 01 25
							if(iLen==1)																			  //15693Э��ʹ�ã�ѡ������
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
							if(iLen==1)																			   //15693Э��ʹ�ã���Ƭ����Ϊ����״̬����
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
							if(iLen==1)																				//15693Э��ʹ�ã�дAFI����
							{																						//xx:AFIֵ
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
							if(iLen==1)																				 //15693Э��ʹ�ã�����AFI����
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
							if(iLen==1)																				  //15693Э��ʹ�ã�дDSFID����
							{																						  //xx:DSFIDֵ
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
							if(iLen==1)																				   //15693Э��ʹ�ã���DSFID����
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
							if(iLen>0)																					//15693Э��ʹ�ã����ϵͳ��Ϣ����
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
							if(iLen == 0x08)																			  //15693Э��ʹ�ã���AFIֵ��������
							{																							  //xx:AFI��ֵ
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
