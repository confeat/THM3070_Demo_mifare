/********************************************************************************
* vim: set ts=4 sw=4 fdm=marker:
*	Project: THM3060
*	File: Crypto1.c
*	Original Version: V1.0
*	Created by DingYM, 2009.4.19
*	Modified by Luqian, 2013.8.27, for THM3060V20_20130425
*	Current Version: V1.1
********************************************************************************/
#include "Crypto1.h"
#include "thm3070.h"
#include "spi.h" 
#include "ISO14443.h"

/********************************Globals**************************************/
unsigned char  M_REQA[2] =     {0x1,0x26};
unsigned char  M_WUPA[2] =     {0x1,0x52};
unsigned char  M_HLTA[2] =     {0x1,0x50};
unsigned char  M_ANTI[3] =     { 0x2,0x93,0x20 }; 
unsigned char  M_SELECT[3] =   { 0x7,0x93,0x70 }; 
unsigned char  M_AUTHA[2] =    { 0x2,0x60 };
unsigned char  M_AUTHB[2] =    { 0x2,0x61 };
unsigned char  M_READ[2]  =    { 0x2,0x30 };
unsigned char  M_WRITE[2] =    { 0x2,0xa0 };
unsigned char  M_DECREMENT[2]= {0x02,0xC0};
unsigned char  M_INCREMENT[2]= {0x2,0xC1};
unsigned char  M_TRANSFER[2] = {0x2,0xB0};          
unsigned char  M_RESTORE[2]  = {0x2,0xC2};
unsigned char  M_temp[20];
unsigned char  M_gUID[5];
unsigned char  M_Len;
unsigned char  M_gKEYA[6];
unsigned char  M_gKEYB[6];
unsigned char  tempno;

  
extern void THM_MSendFrame(unsigned char *buffer,unsigned char num);
extern unsigned char THM_MWaitReadFrame(unsigned char *len, unsigned char *buffer);

/*******************************************************************************
//Function: Change to THM3060 to Mifare Mode and Find& SELECT card
//Parameter: OUT b_uid, card's UID , 4 bytes
//Return value:   00    OK,Mifare card was found and SELECTed
//                01    Err, Not found
//                02    Err, Not 4 UIDs,May be other type
//                03    Err, ATQA Err
//                04    Err, Other  
********************************************************************************/
unsigned char THM_MFindCard(unsigned char * b_uid)
{
    unsigned char iLen;
	unsigned char i,temp;
    //Change to Mifare mode
   THM_WriteReg(PSEL,0x50);
    //Reset Crypto1
    THM_WriteReg(FM_CTRL,0xc0);
    // don't care FDT, don't Encrypt  
    THM_WriteReg(STAT_CTRL,0x00);
    //Short Frame
	THM_WriteReg(FM_CTRL,0x40);   

    THM_MSendFrame(&M_WUPA[1],M_WUPA[0]);     
    THM_MWaitReadFrame(&iLen, M_temp);  

    if (iLen != 2)  //ATQA must equal 2
    {
        //Retry
        THM_MSendFrame(&M_WUPA[1],M_WUPA[0]);     
        THM_MWaitReadFrame(&iLen, M_temp);    
    }    
    if (iLen!= 2)  // ATQA must equal 2 
    {
        if (iLen == 0) return 0x01;   // no response
        return 0x3;                   
    }
     if ( (M_temp[0]& 0xC0)!= 0x00 )     
    {
        return 0x2;                   //not single UID 
    }   
    //Single UID CARD
    //ANTIcollision command,Bit Oriented Frame    
    THM_WriteReg(FM_CTRL,0x46);
    THM_MSendFrame(&M_ANTI[1],M_ANTI[0]); 
    THM_MWaitReadFrame(&iLen, M_temp);          
    if (iLen !=5) return 4;
    
    for (i =0; i< 5;i++)
    {
        M_gUID[i] = M_temp[i];
        *b_uid++ = M_temp[i];
    }    
    //SELECT Cmd,
    M_temp[0] = M_SELECT[1];
    M_temp[1] = M_SELECT[2];
    for (i =0; i< 5;i++)
    {
        M_temp[2+i]= M_gUID[i];        
    }	  
    //Enable CRC
    THM_WriteReg(CRCSEL,0xC1); 
	//Standard Frame                            
	THM_WriteReg(FM_CTRL,0x42);
    THM_MSendFrame(M_temp,M_SELECT[0]); 
    THM_MWaitReadFrame(&iLen, M_temp);  
    if (iLen!=3)
        return 4;
    else 	 
        return 0x0;   
}    

/*******************************************************************************
Function: Load key A or Key B to PCD
Parameter: IN type 60:kyeA 61:keyB    key:key load to PCD
Return value:  
********************************************************************************/
void THM_MInitialKEY(unsigned char  type, unsigned char * key) 
{
    unsigned char i;
	// Load keyA
	if(type == 0x60)
    {
		for (i =0; i<6;i++)
	    {
	        M_gKEYA[i] = *key++;
	    }
	}
	// Load keyB
	else if(type == 0x61)
    {
		for (i =0; i<6;i++)
	    {
	        M_gKEYB[i] = *key++;
	    }
	}
    return;
}

/*******************************************************************************
Function: Mifare Authentication 
Parameter: IN bKeyNo 60:kyeA Authentication 61:keyB Authentication   
			  bBlock:bBlock NO.
Return value:   00    OK,Auth OK
       ,        01    Err, PARA wrong
                02    Err, Get Rand
                03    Auth Error
                04    Other Error 
********************************************************************************/
unsigned char THM_MAuthentication(unsigned char bKeyNo,unsigned char bBlock)
{
    unsigned char iLen;
	unsigned char i;
	unsigned char tmp,temp,buf[5];
    //单次认证
	THM_WriteReg(0x1d,0x00); 
	  
    if ((bKeyNo == 0x00) || (bKeyNo == 0x60))    
    { // KEY A
        M_temp[0] = M_AUTHA[1];
        M_temp[1] = bBlock;
        //Get Rand
        THM_MSendFrame(M_temp,M_AUTHA[0]); 
    }    
    else if ((bKeyNo == 0x01) || (bKeyNo == 0x61))
    { // KEY B
        M_temp[0] = M_AUTHB[1];
        M_temp[1] = bBlock;
        //Get Rand
        THM_MSendFrame(M_temp,M_AUTHB[0]);     
    }
    else 
        return 0x1;
    //取随机数RB并将数据存储在DATA1~DATA4的BUFFER中
    THM_MWaitReadFrame(&iLen, M_temp);  
    if (iLen !=4) 
        return 0x2;
    //Enable Cyrpto1        
    THM_WriteReg(CRYPTO1_CTRL,0x08);        
    //WRITE Rand1
    THM_WriteReg(DATA1,M_temp[0]);
    THM_WriteReg(DATA2,M_temp[1]);
    THM_WriteReg(DATA3,M_temp[2]);
    THM_WriteReg(DATA4,M_temp[3]);
    //将KEY和UID存储在Data0寄存器中备用
    //WRITE Key
    for ( i =0;i <6;i++)
    { 
        if (bKeyNo == 0x00 || bKeyNo == 0x60)
            THM_WriteReg(DATA0,M_gKEYA[i]);
        else
            THM_WriteReg(DATA0,M_gKEYB[i]);    
    }  
    //WRITE UID  
    for ( i =0;i <4;i++)
    {     
        THM_WriteReg(DATA0,M_gUID[i]);
    }    

    //读卡器产生随机数存储在data1~data4buffer中，在RB的后面
	//RNG_STS
	//RNG_CON
	THM_WriteReg(0xb0,0x01);
	i=0;
	while(1)
	{
		temp = THM_ReadReg(0x31);
		if( temp ==1 )
		{
			buf[i++]= THM_ReadReg(0x32);
			if(i>=4)
			break;
		}		
	}
    THM_WriteReg(DATA1,buf[0]);
    THM_WriteReg(DATA2,buf[1]);
    THM_WriteReg(DATA3,buf[2]);
    THM_WriteReg(DATA4,buf[3]);
	THM_WriteReg(0xb0,0x00);
    //启动发送8个认证字节，包含加密后的4个字节和对RA加密的4个字节（好像是和UID运算，具体不详）
    //Authentication
    THM_WriteReg(CRYPTO1_CTRL,0x0C);
    while (1){
    //发送完成后，卡片返回数据，芯片自动对该数据进行解密运算，并将认证结果返回在状态寄存器中
	tmp = THM_ReadReg(UART_STAT); 
	if (tmp & 0xff)
		break;
	}	
	if ((tmp & 0xef) ==0x00)
	{
	    THM_WriteReg(CRYPTO1_CTRL,0x08);
		// Data TRANSFER Encrypt
	    THM_WriteReg(STAT_CTRL,0x01);
	    return 0;
	}    
	else if (tmp & 0x80)
	    return 0x3;
	else
	    return 0x4;  
}    
/*******************************************************************************
Function: Mifare Multiple Authentication 
Parameter: IN bKeyNo 60:kyeA Multiple Authentication 61:keyB Multiple Authentication   
			  bBlock:bBlock NO.
Return value:   00    OK,Auth OK
       ,        01    Err, PARA wrong
                02    Err, Get Rand
                03    Auth Error
                04    Other Error 
********************************************************************************/

unsigned char THM_MAuthenticationT(unsigned char bKeyNo,unsigned char bBlock)
{
    unsigned char iLen;
	unsigned char i;
	unsigned char tmp,temp,buf[5];
    //WRITE Key
    //Add by ymw 
	THM_WriteReg(0x1d,0x01); 
	//Add by ymw
	for ( i =0;i <6;i++)
    { 
        if (bKeyNo == 0x00 || bKeyNo == 0x60)
            THM_WriteReg(DATA0,M_gKEYA[i]);
        else
            THM_WriteReg(DATA0,M_gKEYB[i]);    
    }  
    //WRITE UID  
    for ( i =0;i <4;i++)
    {     
        THM_WriteReg(DATA0,M_gUID[i]);
    }    
    //
	THM_WriteReg(STAT_CTRL,0x01); 
	//Standard Frame                            
	THM_WriteReg(FM_CTRL,0x42); 
    if ((bKeyNo == 0x00) || (bKeyNo == 0x60))    
    { // KEY A
        M_temp[0] = M_AUTHA[1];
        M_temp[1] = bBlock;
        //Get Rand
        THM_MSendFrame(M_temp,M_AUTHA[0]); 

		THM_MWaitReadFrame(&iLen, M_temp);  
	    if (iLen !=4) 
	        return 0x2;
	    THM_WriteReg(STAT_CTRL,0x00);
			
		//Enable Cyrpto1        
	    THM_WriteReg(CRYPTO1_CTRL,0x08);        
	    //WRITE Rand1
	    THM_WriteReg(DATA1,M_temp[0]);
	    THM_WriteReg(DATA2,M_temp[1]);
	    THM_WriteReg(DATA3,M_temp[2]);
	    THM_WriteReg(DATA4,M_temp[3]);
	
		//RNG_STS
		//RNG_CON
		THM_WriteReg(0xb0,0x01);
		i=0;
		while(1)
		{
			temp = THM_ReadReg(0x31);
			if( temp ==1 )
			{
				buf[i++]= THM_ReadReg(0x32);
				if(i>=4)
				break;
			}		
		}
		THM_WriteReg(0xb0,0x00);
	    THM_WriteReg(DATA1,buf[0]);
	    THM_WriteReg(DATA2,buf[1]);
	    THM_WriteReg(DATA3,buf[2]);
	    THM_WriteReg(DATA4,buf[3]);
	    //Authentication
	    THM_WriteReg(CRYPTO1_CTRL,0x0C);
	    while (1){
		tmp = THM_ReadReg(UART_STAT); 
		if (tmp & 0xff)
			break;
		}	
		if ((tmp & 0xef) ==0x00)
		{
		    THM_WriteReg(CRYPTO1_CTRL,0x08);
			// Data TRANSFER Encrypt
		    THM_WriteReg(STAT_CTRL,0x01);
		    return 0;
		}    
		else if (tmp & 0x80)
		    return 0x3;
		else
		    return 0x4;  
    }    
    else if ((bKeyNo == 0x01) || (bKeyNo == 0x61))
    { // KEY B
//        M_temp[0] = M_AUTHB[1];
//        M_temp[1] = bBlock;
//        //Get Rand
//        THM_MSendFrame(M_temp,M_AUTHB[0]);
		  ISO14443_carrier(0);
		  delay_ms(100);
		  ISO14443_carrier(1);
		  delay_ms(100);
		  if((THM_MFindCard(M_gUID))==0) 
		  {
		  	return (THM_MAuthentication(bKeyNo,bBlock));
		  }
		  else
		  return 4;  
    }
    else 
        return 0x1;

}

/*******************************************************************************
Function:  Mifare READ 
Parameter: IN  bBlock  Card Block
           OUT pdat    pointer of the Block data
           OUT bLen    pointer of data length
Return value:   00    OK;
				01    Err	
********************************************************************************/
unsigned char THM_MRead (unsigned char bBlock,unsigned char * pbLen,unsigned char *pdat )
{  
    M_temp[0] = M_READ[1];
    M_temp[1] = bBlock;
    //Send READ Command    
    THM_MSendFrame(M_temp,M_READ[0]);     
    THM_MWaitReadFrame(pbLen, pdat);  

	if (*pbLen != (MBLOCK_SIZE+2))	
		return 1;
	*pbLen= MBLOCK_SIZE;	 //remove 2 CRC bytes
    return 0;
}
      
/*******************************************************************************
Function:  Mifare WRITE 
Parameter: IN  bBlock  Card Block
           OUT pdat    pointer of the Block data
Return value:   00    OK;
                01    WRITE Command Response Error	
********************************************************************************/
unsigned char THM_MWrite(unsigned char bBlock,unsigned char *pdat)
{
    unsigned char iLen;
    
    M_temp[0] = M_WRITE[1];
    M_temp[1] = bBlock;
    
    //WRITE Command    
    THM_MSendFrame(M_temp,M_WRITE[0]);          
    THM_MWaitReadFrame(&iLen, M_temp);
    if (iLen != 1 ||  M_temp[0]!= 0xA0 ) 
		return 0x1;
    //Send WRITE Data
    THM_MSendFrame( pdat, MBLOCK_SIZE);             
    THM_MWaitReadFrame(&iLen, M_temp);   //M_WRITE Command no response
    return 0;
    
}    

/*******************************************************************************
Function:  Mifare TRANSFER 
Parameter: IN  bBlock  Card Block
           OUT pdat    pointer of the Block data
           OUT bLen    pointer of data length
Return value:   00    OK;
                01    Err,response	
********************************************************************************/
unsigned char THM_MTRANSFER(unsigned char bBlock)
{
    unsigned char iLen;
    
    M_temp[0] = M_TRANSFER[1];
    M_temp[1] = bBlock;
    
    //TRANSFER Command    
    THM_MSendFrame(M_temp,M_TRANSFER[0]);         
    THM_MWaitReadFrame(&iLen, M_temp);
    if (iLen != 1 || M_temp[0] != 0xA0 ) return 0x1;    
    return 0;    
}    

/*******************************************************************************
Function:  Mifare RESTORE 
Parameter: IN  bBlock  Card Block
Return value:   00    OK;
                01    Err,Response	
********************************************************************************/
unsigned char THM_MRESTORE(unsigned char bBlock)
{
    unsigned char iLen;
    
    M_temp[0] = M_RESTORE[1];
    M_temp[1] = bBlock;
    
    //WRITE Command    
    THM_MSendFrame(M_temp,M_RESTORE[0]);         
    THM_MWaitReadFrame(&iLen, M_temp);
    
    if (iLen != 1 || M_temp[0] != 0xA0 ) return 0x1;    
    //Send Data，Data have no Meaning
    THM_MSendFrame( M_temp, MWALLET_SIZE);//              
    THM_MWaitReadFrame(&iLen, M_temp);   //M_RESTORE Command no response
    return 0;
    
}    

/*******************************************************************************
Function:  Mifare INCREMENT 
Parameter: IN  bBlock  Card Block
           IN  pdat  INCREMENT value ( 4 bytes )
Return value:   00    OK;
                01    Err,Response	
********************************************************************************/
unsigned char THM_MINCREMENT(unsigned char bBlock,unsigned char *pbValue )
{
    unsigned char iLen;
    
    M_temp[0] = M_INCREMENT[1];
    M_temp[1] = bBlock;
    
    //INCREMENT Command    
    THM_MSendFrame(M_temp,M_INCREMENT[0]);         
    THM_MWaitReadFrame(&iLen, M_temp);
    
    if (iLen != 1 || M_temp[0] != 0xA0 ) return 0x1;    
    
    //Send INCREMENT Data
    THM_MSendFrame( pbValue, MWALLET_SIZE);//              
    THM_MWaitReadFrame(&iLen,M_temp );   //no response
    return 0;
    
}    

/*******************************************************************************
Function:  Mifare DECREMENT 
Parameter: IN  bBlock  Card Block
           IN  pdat  INCREMENT value ( 4 bytes )
Return value:   00    OK;
                01    Err,Response	
********************************************************************************/
unsigned char THM_MDECREMENT(unsigned char bBlock,unsigned char *pbValue )
{
    unsigned char iLen;
    
    M_temp[0] = M_DECREMENT[1];
    M_temp[1] = bBlock;
    
    //DECREMENT Command    
    THM_MSendFrame(M_temp,M_DECREMENT[0]);         
    THM_MWaitReadFrame(&iLen, M_temp);
    
    if (iLen != 1 || M_temp[0] != 0xA0 ) return 0x1;    
    
    //Send DECREMENT Data
    THM_MSendFrame( pbValue, MWALLET_SIZE); //              
    THM_MWaitReadFrame(&iLen, M_temp);     //no response
    return 0;
    
} 

/*******************************************************************************
Function:  Mifare DECREMENT or INCREMENT
Parameter: IN  type   C1 DECREMENT or C0 INCREMENT
		   IN  bBlock Card Block
           IN  pdat   INCREMENT value ( 4 bytes )
Return value:   00    OK;
                01    Err,Response	
********************************************************************************/
unsigned char THM_Mchange(unsigned char type,unsigned char bBlock,unsigned char *pbValue )
{
	if(type == 0xC1)
	{
		THM_MINCREMENT(bBlock,pbValue );	
	}
	else if (type == 0xC0)
	{
		THM_MDECREMENT(bBlock,pbValue );
	}
	THM_MTRANSFER(bBlock);
	return 0;   
}   
  
/*******************************************************************************
Function: Mifare WRITE Value
Parameter: IN  bBlock  Card Block
           OUT pdat    pointer of the Block data
Return value:   00    OK;
                01    WRITE Command Response Error	
********************************************************************************/
unsigned char THM_MWriteValue(unsigned char bBlock,unsigned char * pbValue,unsigned char bAddr)
{
    
    unsigned char i;
    //按照value  ~value  value  ~value  address  ~address  address  ~address对将要写入的数据进行编码
    //BYTE0~3 Value   BYTE4~7 !Value BYTE 8~11 Value BYTE 12 Addr BYTE 13 !Addr BYTE 14 Addr BYTE 15 !addr
    //Combine Value blcok
    for (i=0;i<4;i++)
    {                 
        M_temp[i+2] = pbValue[i];
        M_temp[i+6] = ~pbValue[i];
        M_temp[i+10]=pbValue[i];
    }    
    M_temp[14] = bAddr;
    M_temp[15] = ~bAddr;
    M_temp[16] = bAddr;
    M_temp[17] = ~bAddr;         
    return(THM_MWrite(bBlock,M_temp+2));    
}    

/*******************************************************************************
Function: Mifare READ Value
Parameter: IN  bBlock  Card Block
           OUT pbValue    pointer of the Block data
           OUT pbAddr     pointer of Addr
Return value:   00    OK;
                01    Value format Error    
                02    Addr format Error
                03    Other Error	
********************************************************************************/
unsigned char THM_MReadValue(unsigned char bBlock,unsigned char * pbValue,unsigned char * pbAddr)
{
	unsigned char i;
	unsigned char temp[4];
	//READ bBlock data
	if ( THM_MRead (bBlock,&i,M_temp) != 0x00)	 	
	return 0x3;  //
   // 读取数据结果value ~value  value 相互之间做比较  
	for (i =0;i < 4;i++)
	{
		
		if (M_temp[i]!= M_temp[i+8])
			return 0x1;       //Value format error
		temp[0] = ~M_temp[i+4];
		//if (M_temp[i]!=~M_temp[i+4])
		if (M_temp[i]!=temp[0])
			return 0x1;
		pbValue[i] = M_temp[i];
	}

    // Address  ~address  address ~address之间相互比较
	if (M_temp[12]!=M_temp[14]) return 0x2; //Addr format err
	if (M_temp[13]!=M_temp[15])	return 0x2;
	temp[0] = ~M_temp[13];
	//if (M_temp[12]!=(~M_temp[13]))return 0x2;
	if (M_temp[12]!=temp[0])return 0x2;
	*pbAddr = M_temp[12];

	return 0x00;  //Success
}

/*******************************************************************************
Function: Mifare WRITE Sector Trailer
Parameter: IN  bBlock  Card Block, Must be Block 3 in the Sector
           IN  pbKEYA   
           IN  pbKEYB
           IN  pbAccessBits              
           OUT pdat   pointer of the Block data
Return value:   00    OK;
                01    Err,WRITE Command Response Error    
                02    Err,Block No Error
                03    Err,Access bits Error
bAccessBits
BYTE 6
 !C2_3 !C2_2 !C2_1 !C2_0 !C1_3 !C1_2 !C1_1 !C1_0
BYTE 7
 C1_3 C1_2 C1_1 C1_0 !C3_3 !C3_2 !C3_1 !C3_0
BYTE 8
 C3_3 C3_2 C3_1 C3_0 C2_3 C2_2 C2_1 C2_0
BYTE 9,not used	
********************************************************************************/
unsigned char THM_MWriteSectorTrailer(unsigned char bBlock,unsigned char* pbKEYA, unsigned char * pbKEYB, unsigned char * pbAccessBits)
{
    unsigned char i;
    //该操作只针对Block3  且对Block的第6 7 8 三个字节进行判断   
    // Block No. must be highest block of the sector
    if ((bBlock & 0x3)!= 0x3) return 0x2;
    // BYTE6 Low with BYTE7 High
    if ((~pbAccessBits[0] & 0xF)!=((pbAccessBits[1] & 0xF0) >> 4))  return 0x3;
    // BYTE6 High with BYTE8 Low
    if (((~pbAccessBits[0] & 0xF0) >> 4) !=(pbAccessBits[2] & 0xF)) return 0x3;
    //BYTE7 Low with BYTE8 High
    if ((~pbAccessBits[1] & 0xF)!=((pbAccessBits[2] & 0xF0) >> 4))  return 0x3; 
	 
    //修改密钥和新的控制字节    
    //Combine SectorTrailer block
    for (i=0;i<6;i++)
    {
        M_temp[i+2] =  *pbKEYA++;
        M_temp[i+12] = *pbKEYB++;
    }         
    
    M_temp[8] =  pbAccessBits[0]; 
    M_temp[9] =  pbAccessBits[1];
    M_temp[10] = pbAccessBits[2];
    M_temp[11] = pbAccessBits[3];
    
    return(THM_MWrite(bBlock,M_temp+2)); 
}      
  
/*******************************************************************************
Function: Mifare HLTA
Parameter: 
Return value:   	
********************************************************************************/
void THM_MHLTA(void)
{
    unsigned char iLen;
	//Send M_HLTA command

    THM_MSendFrame(&M_HLTA[1],M_HLTA[0]); 
    THM_MWaitReadFrame(&iLen, M_temp);        
	return;  
}  
  
/*******************************************************************************
* Function    : THM_SendFrame
* Description : Send Value of Databuffer to PICC	
* Arguments   : buffer  --- send date buffer
				num     --- recieve date length			
* Return      :	
*******************************************************************************/
extern void THM_MSendFrame(unsigned char *buffer,unsigned char num)
{
	unsigned char temp;	
	THM_WriteReg(SCNTL, 0x5);	                                //RAMPT_CLR =1,CLK_EN =1
	THM_WriteReg(SCNTL, 0x01);                                  // RAMPT_CLK=0;	
	
	temp = DATA | 0x80;			                                //write mode	
		                        								 
	SPI_FRAME_START();
	
	SPI_SendBuff(&temp,1);
	SPI_SendBuff(buffer,num);	                                //write data	
																
	SPI_FRAME_END();
	
	THM_WriteReg(SND_CTRL, 0x01);                               // SEND =1 ，send start		    
}

/*******************************************************************************
* Function    : THM3070 WaitReadFrame
* Description : Wait recieve ended,read data and save to buffer
* Arguments   : buffer  --- recieve date buffer
				len     --- recieve date length			
* Return      :	01      --- recieve end 
				other   --- recieve error
*******************************************************************************/
extern unsigned char THM_MWaitReadFrame(unsigned char *len, unsigned char *buffer)
{
	unsigned char temp,temp1;	
	*len =0;
	//waiting the end of receive
	while (1)	
	{  	    
	   temp = THM_ReadReg(UART_STAT);   
	   if (temp & 0xFF)
	        break;
    }	           
	//handling the receiving status
    if (temp & 0xEF) 
	{
	    THM_WriteReg(UART_STAT,0x00);
		return (temp & 0xEF);		
	}
	//read the data length
    *len =((unsigned char)(THM_ReadReg(RSCH)) <<8 ) + THM_ReadReg(RSCL);  		    
	//read the data
    if (*len != 0x00 )
    {
        SPI_FRAME_START();
        
        temp1 = DATA;
        SPI_SendBuff( &temp1,1);
        SPI_RecvBuff( buffer,*len);
        
        SPI_FRAME_END();
    }      
      
    return (temp);
}

