/********************************************************************************
* vim: set ts=4 sw=4 fdm=marker:
*	Project: THM3060
*	File: Thm3070.c
*	Original Version: V1.0
*	Created by DingYM, 2009.4.19
*	Modified by Luqian, 2013.8.27, for THM3060V20_20130425
*	Current Version: V1.1
********************************************************************************/

#include "ISO14443.h"
#include "thm3070.h"
#include "SPI.h"

 char globle=0;
  char globle1=0;
/*******************************************************************************
* Function    : THM_EnableEMV
* Description : EMV_Enable
* Arguments   :  			
* Return      :
*******************************************************************************/
void THM_EnableEMV(void)
{
	THM_WriteReg(EMVEN, 0xfd);
}

/*******************************************************************************
* Function    : THM_DisableEMV
* Description : EMV_Disable
* Arguments   :  			
* Return      :
*******************************************************************************/
void THM_DisableEMV(void)
{
	THM_WriteReg(EMVEN, 0x00);
}

/*******************************************************************************
* Function    : THM_WriteReg
* Description : Write Register
* Arguments   :  			
* Return      :
*******************************************************************************/
void THM_WriteReg(unsigned char address,unsigned char content)
{
	u8 temp_buff[2];
	// BIT7 =1	
	temp_buff[0] = address | 0x80;
	temp_buff[1] = content;
	// Frame Write
	SPI_FRAME_START();	
	SPI_SendBuff(temp_buff,2);	
	SPI_FRAME_END();
}

/*******************************************************************************
* Function    : THM_ReadReg
* Description : Read Register
* Arguments   :  			
* Return      :
*******************************************************************************/
u8 THM_ReadReg(unsigned char address)
{
	unsigned char temp_buff[1];
	// SS_N =0;
	SPI_FRAME_START();	
	// BIT7 = 0;
	temp_buff[0] = address & 0x7F;	
	SPI_SendBuff(temp_buff,1);
	SPI_RecvBuff(temp_buff,1);
	//SS_N =1;	
	SPI_FRAME_END();
	return(temp_buff[0]);
}

/*******************************************************************************
* Function    : THM_WaitReadFrame
* Description : Wait recieve ended,read data and save to buffer
* Arguments   : buffer  --- recieve date buffer
				len     --- recieve date length			
* Return      :	01      --- recieve end 
				other   --- recieve error
*******************************************************************************/
u8 THM_WaitReadFrame(unsigned short *len, unsigned char *buffer)
{
	unsigned char temp,temp1;	
	unsigned char bEmvErr=0;
	// recieve end
	while (1)	
	{ 
		bEmvErr = (THM_ReadReg(EMVERR)&0x3b);
		temp = THM_ReadReg(RSTAT);   
		globle = temp;
		globle1  = THM_ReadReg(EMVERR);
		if (temp)
		{
			if(bEmvErr&FDTPICCNOISEERR)
				return FERR;					
			break;
		}
    }	           

    if (temp & CERR )  
	{ 
        temp = CERR;                //Collision  error
	    delay_ms(10);				//wait until send over
	}
    else if (temp & PERR)       
        temp = PERR;                //Parity error
    else if (temp & FERR)       
        temp = FERR;
    else if (temp & DATOVER)       
        temp = DATOVER;            //Datavoer error
    else if (temp & CRCERR)   
        temp = CRCERR;              //CRC error
    else if (temp & TMROVER)
        temp = TMROVER;             //Time out error
    else 
        temp = FEND;                //Frame end

	//Length of recieve data
	*len = (THM_ReadReg(RSCH)<<8)+THM_ReadReg(RSCL);  	
	if(*len>255)
	return (temp);		
	//Read data 
	if (*len != 0x00 )
	{
	    SPI_FRAME_START();
	        
	    temp1 = DATA;
	    SPI_SendBuff( &temp1,1);
	    SPI_RecvBuff( buffer,*len);
	        
	    SPI_FRAME_END();
	} 
     
    //Clear status
    THM_WriteReg(RSTAT,0x00);        
    return (temp);
}

/*******************************************************************************
* Function    : THM_SendFrame
* Description : Send Value of Databuffer to PICC	
* Arguments   : buffer  --- send date buffer
				num     --- recieve date length			
* Return      :	
*******************************************************************************/ 
void THM_SendFrame(unsigned char *buffer,unsigned short num)
{
	unsigned char temp;	
	THM_WriteReg(SCNTL, 0x5);	                                //RAMPT_CLR =1,CLK_EN =1
	THM_WriteReg(SCNTL, 0x01);                                  // RAMPT_CLK=0;	
	THM_WriteReg(EMVERR,0xff);
	
	temp = DATA | 0x80;			                                //Write Command
	                        
	SPI_FRAME_START();
	
	SPI_SendBuff(&temp,1);
	SPI_SendBuff(buffer,num);	                                 //Write Date	
	
	SPI_FRAME_END();
	
	THM_WriteReg(SCNTL, 0x03);                                  // SEND =1 £¬send start
	while(!THM_ReadReg(TXFIN));
}

/*******************************************************************************
* Function    : THM_ChangeProtBaud
* Description : Change  Type&&Sendband&&Recband	
* Arguments   : prot    --- Type
				sndbaud --- Sendband
				rcvbaud	--- Recband		
* Return      :	
*******************************************************************************/
void THM_ChangeProtBaud(unsigned char prot, unsigned char sndbaud, unsigned char rcvbaud)
{
    THM_WriteReg( PSEL, prot | sndbaud | rcvbaud );
    return;
}    

/*******************************************************************************
* Function    : THM_Reset
* Description : THM3070  GPIO initial	
* Arguments   : 		
* Return      :	
*******************************************************************************/
void THM_Reset(void)
{
	SPI_Init_GPIO();
	THM_En_2Tx();		            //Config init
}

/*******************************************************************************
* Function    : THM_SetFWT
* Description : Set FWT	
* Arguments   : ulFWT --  2^FWI * WTXM		
* Return      :	
*******************************************************************************/
void THM_SetFWT(u32 ulFWT)
{
	u32 fwt = ulFWT+1; 
	THM_WriteReg(FWTH, fwt>>16);
	THM_WriteReg(FWTM, fwt>>8);
	THM_WriteReg(FWTL, fwt&0xFF);
}

/*******************************************************************************
* Function    : SleepMode
* Description : Mode control of THM3070
* Arguments   : Modetype --  0:  Sleep Mode 
*							 1:  Normol Mode		
* Return      :	
*******************************************************************************/
void SleepMode(u8 Modetype)
{
	if (0 == Modetype)
		GPIO_SetBits(GPIOB,GPIO_Pin_1);    // Set PB1 as STANDBY and set SSN high	
	else
		GPIO_ResetBits(GPIOB,GPIO_Pin_1);    // Set PB1 as STANDBY and set SSN LOW	
}
