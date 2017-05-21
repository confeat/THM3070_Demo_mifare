/********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: ISO15693.c
* Author: Lichong
* Version: V1.0
* History:
*   2015-08-24 Original version
********************************************************************************/
#include "ISO14443.h"
#include "thm3070.h"
#include "string.h"
#include "SPI.h"
#include "EMV_Main.h"

#define Bit0_En 0x01
u8 gUID[8];
u8 VCDbuf[256];

/*******************************************************************************
* Function    : ISO15693_ReadCARD_ReadCARD
* Description : ISO15693 Inventory
* Arguments   : * buf  ---  UID 		
* Return      :	UID length
*******************************************************************************/
u16 ISO15693_ReadCARD_WithAFI(u8 * buf,u8 AFI)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp,temp1;
	unsigned char i,temp_buff[255];
	unsigned char collisionflag = 0;
	unsigned char masklengthB = 0;
	unsigned char masklengthb = 0;
	unsigned char mask[8] = {0};
	unsigned char dsfidcollisionflag = 0;


	//15693
	THM_WriteReg(0x01,0x20);
	THM_WriteReg(0x10,0x01);
    //05
   	comd_buff[0]=0x36;
	comd_buff[1]=0x01;
	comd_buff[2]=AFI;
   	comd_buff[3]=0x00;
   	temp = 4;
	// 发送清点命令 26 01 00  所有在场VICC返回UID
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	// 如果没有冲突返回UID
	if(cRes == 0x01)
	{	memcpy(buf, temp_buff+2,0x08);
		memcpy(gUID, temp_buff+2,0x08);
		return 0x08;
	}
	//如果发生冲突，判断是够由DSFID引起，如果是 返回错误 否则认为是UID冲突，记录当前冲突字节和bit
	else if	(cRes == 0x40)
	{
	//UID冲突  记录冲突发生的字节和BIT位，由此计算MASK Length，并将接收到的数据赋值给MASK，方便下次清点
		if(temp>2)
		{
			collisionflag =1;
			temp1 = THM_ReadReg (BPOS)+1;
			masklengthB = temp-2;
			masklengthb = ((masklengthB-1)<<3)+ temp1;
			for(i=0;i<=masklengthB;i++)
			{
				mask[i]=temp_buff[2+i];
			}
		}
	//DSFID冲突，Mask Length=1
		else
		{
			collisionflag =1;
			dsfidcollisionflag = 1;
			masklengthb =1;
		}
				
	}
	//如果是其他错误返回0
	else
		return 0;
   //冲突后继续发送清点命令，掩码和掩码长度根据上一次的冲突来确定，
	while(collisionflag)
	{
		delay_ms(10);
		// 冲突由UID引起
		if(dsfidcollisionflag == 0)
		{
		   	comd_buff[0]=0x36;
			comd_buff[1]=0x01;
			comd_buff[2]=AFI;
	   		comd_buff[3]=masklengthb;
			for(i=0;i<masklengthB;i++)
			{
				comd_buff[4+i] = mask[i];
			} 
	   		temp = 4+masklengthB;
			THM_SendFrame(comd_buff,temp);
			cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
			//接收正确
			if(cRes == 0x01)
			{	memcpy(buf, temp_buff+2,0x08);
				memcpy(gUID, temp_buff+2,0x08);
				collisionflag =0;
				return 8;			 
			}
			else if	(cRes == 0x40)
			{
			//UID冲突，	 记录冲突发生的字节和BIT位
				if(temp>2)
				{
					collisionflag =1;
					temp1 = THM_ReadReg (BPOS)+1;
					masklengthB = temp-2;
					masklengthb = ((masklengthB-1)<<3)+ temp1;
					for(i=0;i<=masklengthB;i++)
					{
						mask[i]=temp_buff[2+i];
					}
				}
				else
				{
					collisionflag =0;
					return 0;
				}
			}
			else
			{
				collisionflag =0;
				return 0;
			}
		}
		//冲突由DSFID引起
		else
		{

		   	while(dsfidcollisionflag&&(masklengthb<64))
			{
		   		delay_ms(10);
				//Mask最高有效位为0
				comd_buff[0]=0x36;
				comd_buff[1]=0x01;
				comd_buff[2]=AFI;
			   	comd_buff[3]=masklengthb;
				for(i=0;i<=masklengthB;i++)
				{
					comd_buff[4+i] = mask[i];
				} 
		   		temp = 5+masklengthB;
				THM_SendFrame(comd_buff,temp);
				cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
				//接收正确
				if(cRes == 0x01)
				{	memcpy(buf, temp_buff+2,0x08);
					memcpy(gUID, temp_buff+2,0x08);
					collisionflag =0;
					dsfidcollisionflag = 0;
					return 8;
				}
				//冲突后Mask Length加1
				else if	(cRes == 0x40)
				{
					collisionflag =1;
					masklengthb+=1;
					if((masklengthb%8)==0)
						 masklengthB += 1;				
				}
				else
				{
				////Mask最高有效位为1
					mask[masklengthB]+=(Bit0_En<<((masklengthb-1)%8));
					comd_buff[0]=0x36;
					comd_buff[1]=0x01;
					comd_buff[2]=AFI;
				   	comd_buff[3]=masklengthb;
					for(i=0;i<=masklengthB;i++)
					{
						comd_buff[4+i] = mask[i];
					} 
			   		temp = 5+masklengthB;
					THM_SendFrame(comd_buff,temp);
					cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
					//接收正确
					if(cRes == 0x01)
					{	memcpy(buf, temp_buff+2,0x08);
						memcpy(gUID, temp_buff+2,0x08);
						collisionflag =0;
						dsfidcollisionflag = 0;
						return 8;
					}
					//冲突后Mask Length加1
					else if	(cRes == 0x40)
					{
						collisionflag =1;
						masklengthb+=1;
						if(((masklengthb-1)%8)==0)
							 masklengthB += 1;				
					}
					//其他错误，结束
					else
					{ 	
						collisionflag =0;
						dsfidcollisionflag = 0;
						return 0;

					}
				}
			}
		}
	}	
}



/*******************************************************************************
* Function    : ISO15693_ReadCARD_ReadCARD
* Description : ISO15693 Inventory
* Arguments   : * buf  ---  UID 		
* Return      :	UID length
*******************************************************************************/
u16 ISO15693_ReadCARD(u8 * buf)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp,temp1;
	unsigned char i,temp_buff[255];
	unsigned char collisionflag = 0;
	unsigned char masklengthB = 0;
	unsigned char masklengthb = 0;
	unsigned char mask[8] = {0};
	unsigned char dsfidcollisionflag = 0;


	//15693
	THM_WriteReg(0x01,0x20);
	THM_WriteReg(0x10,0x01);
    //05
   	comd_buff[0]=0x26;
	comd_buff[1]=0x01;
   	comd_buff[2]=0x00;
   	temp = 3;
	// 发送清点命令 26 01 00  所有在场VICC返回UID
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	// 如果没有冲突返回UID
	if(cRes == 0x01)
	{	memcpy(buf, temp_buff+2,0x08);
		memcpy(gUID, temp_buff+2,0x08);
		return 0x08;
	}
	//如果发生冲突，判断是够由DSFID引起，如果是 返回错误 否则认为是UID冲突，记录当前冲突字节和bit
	else if	(cRes == 0x40)
	{
	//UID冲突  记录冲突发生的字节和BIT位，由此计算MASK Length，并将接收到的数据赋值给MASK，方便下次清点
		if(temp>2)
		{
			collisionflag =1;
			temp1 = THM_ReadReg (BPOS)+1;
			masklengthB = temp-2;
			masklengthb = ((masklengthB-1)<<3)+ temp1;
			for(i=0;i<=masklengthB;i++)
			{
				mask[i]=temp_buff[2+i];
			}
		}
	//DSFID冲突，Mask Length=1
		else
		{
			collisionflag =1;
			dsfidcollisionflag = 1;
			masklengthb =1;
		}
				
	}
	//如果是其他错误返回0
	else
		return 0;
   //冲突后继续发送清点命令，掩码和掩码长度根据上一次的冲突来确定，
	while(collisionflag)
	{
		delay_ms(10);
		// 冲突由UID引起
		if(dsfidcollisionflag == 0)
		{
		   	comd_buff[0]=0x26;
			comd_buff[1]=0x01;
	   		comd_buff[2]=masklengthb;
			for(i=0;i<masklengthB;i++)
			{
				comd_buff[3+i] = mask[i];
			} 
	   		temp = 3+masklengthB;
			THM_SendFrame(comd_buff,temp);
			cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
			//接收正确
			if(cRes == 0x01)
			{	memcpy(buf, temp_buff+2,0x08);
				memcpy(gUID, temp_buff+2,0x08);
				collisionflag =0;
				return 8;			 
			}
			else if	(cRes == 0x40)
			{
			//UID冲突，	 记录冲突发生的字节和BIT位
				if(temp>2)
				{
					collisionflag =1;
					temp1 = THM_ReadReg (BPOS)+1;
					masklengthB = temp-2;
					masklengthb = ((masklengthB-1)<<3)+ temp1;
					for(i=0;i<=masklengthB;i++)
					{
						mask[i]=temp_buff[2+i];
					}
				}
				else
				{
					collisionflag =0;
					return 0;
				}
			}
			else
			{
				collisionflag =0;
				return 0;
			}
		}
		//冲突由DSFID引起
		else
		{

		   	while(dsfidcollisionflag&&(masklengthb<64))
			{
		   		delay_ms(10);
				//Mask最高有效位为0
				comd_buff[0]=0x26;
			   	comd_buff[2]=masklengthb;
				for(i=0;i<=masklengthB;i++)
				{
					comd_buff[3+i] = mask[i];
				} 
		   		temp = 4+masklengthB;
				THM_SendFrame(comd_buff,temp);
				cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
				//接收正确
				if(cRes == 0x01)
				{	memcpy(buf, temp_buff+2,0x08);
					memcpy(gUID, temp_buff+2,0x08);
					collisionflag =0;
					dsfidcollisionflag = 0;
					return 8;
				}
				//冲突后Mask Length加1
				else if	(cRes == 0x40)
				{
					collisionflag =1;
					masklengthb+=1;
					if((masklengthb%8)==0)
						 masklengthB += 1;				
				}
				else
				{
				////Mask最高有效位为1
					mask[masklengthB]+=(Bit0_En<<((masklengthb-1)%8));
					comd_buff[0]=0x26;
					comd_buff[1]=0x01;
				   	comd_buff[2]=masklengthb;
					for(i=0;i<=masklengthB;i++)
					{
						comd_buff[3+i] = mask[i];
					} 
			   		temp = 4+masklengthB;
					THM_SendFrame(comd_buff,temp);
					cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
					//接收正确
					if(cRes == 0x01)
					{	memcpy(buf, temp_buff+2,0x08);
						memcpy(gUID, temp_buff+2,0x08);
						collisionflag =0;
						dsfidcollisionflag = 0;
						return 8;
					}
					//冲突后Mask Length加1
					else if	(cRes == 0x40)
					{
						collisionflag =1;
						masklengthb+=1;
						if(((masklengthb-1)%8)==0)
							 masklengthB += 1;				
					}
					//其他错误，结束
					else
					{ 	
						collisionflag =0;
						dsfidcollisionflag = 0;
						return 0;

					}
				}
			}
		}
	}	
}
/*******************************************************************************
* Function    : ISO15693_Stayquiet
* Description : ISO15693_Stayquiet
* Arguments   : 		
* Return      :	1  
*******************************************************************************/
u16 ISO15693_Stayquiet(void)
{
	unsigned char comd_buff[255];
	unsigned short temp;

	comd_buff[0]=0x22;
	comd_buff[1]=0x02;
	memcpy(comd_buff+2, gUID,0x08);

	temp = 0x0a;
	
	THM_SendFrame(comd_buff,temp);

	return 1;
}

/*******************************************************************************
* Function    : ISO15693_ReadBlock
* Description : ReadBlock
* Arguments   : Blocknum  --the block N0. to be read
				* buf     -- data returned
* Return      :	data length  
*******************************************************************************/
u16 ISO15693_ReadBlock(u8 Blocknum ,u8 * buf)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x20;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=Blocknum;

	temp = 0x0b;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
	{	
		memcpy(buf, temp_buff,temp);
		return temp;;
	}
	else 
		return 0;
}

/*******************************************************************************
* Function    : ISO15693_WriteBlock
* Description : WriteBlock
* Arguments   : Blocknum  --the block N0. to be write
				datalen   --block size 
				* buf     -- data  
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_WriteBlock(u8 Blocknum ,u8 datalen,u8 * buf)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x21;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=Blocknum;
	memcpy(comd_buff+0x0b, buf,datalen);
	temp = (0x0b+datalen);
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;
}
/*******************************************************************************
* Function    : ISO15693_LockBlock
* Description : LockBlock  and the block will not be write
* Arguments   : Blocknum  --the block N0. to be locked
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_LockBlock(u8 Blocknum)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x22;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=Blocknum;
	temp = 0x0b;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;
}
/*******************************************************************************
* Function    : ISO15693_ReadMBlock
* Description : Read   n  Block
* Arguments   : Blockstart  --the  start  block	  number
				Blocklen    --the number will be read
				* buf       --data
* Return      :	data length
*******************************************************************************/
u16 ISO15693_ReadMBlock(u8 Blockstart ,u8 Blocklen,u8 * buf)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];


	comd_buff[0]=0x22;
	comd_buff[1]=0x23;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=Blockstart;
	comd_buff[0x0b]=Blocklen;

	temp = 0x0c;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
	{	
		memcpy(buf, temp_buff,temp);
		return temp;
	}
	else 
		return 0;

}
/*******************************************************************************
* Function    : ISO15693_WriteMBlock
* Description : Write   n  Block
* Arguments   : Blockstart  --the  start  block	  number
				Blocklen    --the number will be write
				* buf       --data
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_WriteMBlock(u8 Blockstart ,u8 Blocknum,u8 datalen,u8 * buf)
{
//	unsigned char cRes = 0;
//	unsigned char comd_buff[255];
//	unsigned short temp;
//	unsigned char temp_buff[255];
//
//	comd_buff[0]=0x22;
//	comd_buff[1]=0x24;
//	memcpy(comd_buff+2, gUID,0x08);
//	comd_buff[0x0a]=Blockstart;
//	comd_buff[0x0b]=Blocknum;
//	memcpy(comd_buff+0x0c, buf,datalen);
//	temp = (0x0c+datalen);
//	
//	THM_SendFrame(comd_buff,temp);
//	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
//	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
//		return 1;	
//	else 
//		return 0;


	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x24;
//	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[2]=Blockstart;
	comd_buff[3]=Blocknum;
	memcpy(comd_buff+4, buf,datalen);
	temp = (4+datalen);
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;	
	else 
		return 0;
}
/*******************************************************************************
* Function    : ISO15693_Select
* Description : Select one vicc to Select status
* Arguments   : 
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_Select(void)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];


	comd_buff[0]=0x22;
	comd_buff[1]=0x25;
	memcpy(comd_buff+2, gUID,0x08);

	temp = 0x0a;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;

}
 /*******************************************************************************
* Function    : ISO15693_Reset2ready
* Description : Set vicc to ready status
* Arguments   : 
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_Reset2ready(void)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];


	comd_buff[0]=0x22;
	comd_buff[1]=0x26;
	memcpy(comd_buff+2, gUID,0x08);

	temp = 0x0a;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))	
		return 1;
	else 
		return 0;
}
 /*******************************************************************************
* Function    : ISO15693_WriteAFI
* Description : WriteAFI
* Arguments   :  AFI
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_WriteAFI(u8 AFI)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x27;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=AFI;
	temp = 0x0b;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;
}
 /*******************************************************************************
* Function    : ISO15693_LockAFI
* Description : LockAFI
* Arguments   : 
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_LockAFI(void)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x28;
	memcpy(comd_buff+2, gUID,0x08);
	temp = 0x0a;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;
}
 /*******************************************************************************
* Function    : ISO15693_WriteDSFID
* Description : WriteDSFID
* Arguments   :  DSFID
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_WriteDSFID(u8 DSFID)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x29;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=DSFID;
	temp = 0x0b;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;
}

 /*******************************************************************************
* Function    : ISO15693_LockDSFID
* Description : LockDSFID
* Arguments   : 
* Return      :	1  sucess
				0  failed 
*******************************************************************************/
u16 ISO15693_LockDSFID(void)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x2A;
	memcpy(comd_buff+2, gUID,0x08);
	temp = 0x0a;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
		return 1;
	else 
		return 0;
}
 /*******************************************************************************
* Function    : ISO15693_GetSysInf
* Description : ISO15693_GetSysInf
* Arguments   : * buf	  -- System information
* Return      :	length of  	System information
*******************************************************************************/
u16 ISO15693_GetSysInf(u8 * buf)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];

	comd_buff[0]=0x22;
	comd_buff[1]=0x2B;
	memcpy(comd_buff+2, gUID,0x08);
	temp = 0x0a;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
	{
		memcpy(buf, temp_buff,temp);
		return temp;
	}
	else 
		return 0;
}
/*******************************************************************************
* Function    : ISO15693_ReadMBlockStatus
* Description : Read n Block Status
* Arguments   : Blockstart -- Blockstart number
				blocklen   -- number of block 
				* buf	  -- blockstatus
* Return      :	length of  blockstatus
*******************************************************************************/
u16 ISO15693_ReadMBlockStatus(u8 Blockstart ,u8 Blocklen,u8 * buf)
{
	unsigned char cRes = 0;
	unsigned char comd_buff[255];
	unsigned short temp;
	unsigned char temp_buff[255];


	comd_buff[0]=0x22;
	comd_buff[1]=0x2C;
	memcpy(comd_buff+2, gUID,0x08);
	comd_buff[0x0a]=Blockstart;
	comd_buff[0x0b]=Blocklen;

	temp = 0x0c;
	
	THM_SendFrame(comd_buff,temp);
	cRes = THM_WaitReadFrame(&temp,temp_buff);  //Receive from CARD
	if((cRes == 0x01)&&(temp_buff[0] == 0x00))
	{	
		memcpy(buf, temp_buff,temp);
		return temp;
	}
	else 
		return 0;

}
