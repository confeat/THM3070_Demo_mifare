/********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: ISO14443.c
* Author: Li Chong
* Version: V1.0
* History:
*   2014-11-23 Original version
********************************************************************************/

#include "usb_lib.h"
#include "SPI.h"

/*******************************************************************************
* Function    : SPI_FRAME_START
* Description : SSN Control  as LOW
* Arguments   :  			
* Return      :	
*******************************************************************************/ 
void SPI_FRAME_START(void)  
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);                // Set PA4 as SPI_SSN and set SSN LOW
}

/*******************************************************************************
* Function    : SPI_FRAME_END
* Description : SSN Control  as HIGH
* Arguments   :  			
* Return      :	
*******************************************************************************/ 
void SPI_FRAME_END(void)  
{
	GPIO_SetBits(GPIOA,GPIO_Pin_4);                   // Set PA4 as SPI_SSN and set SSN High
}

/*******************************************************************************
* Function    : _send_byte
* Description : Send  one Byte
* Arguments   : dat to be send  			
* Return      :	
*******************************************************************************/ 
void _send_byte(unsigned char dat)
{
   unsigned char i;
   for (i =0;i<8;i++)
   {
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);               // Set PA5 as SPI_CLK and set LOW
		if (dat & 0x80)
			GPIO_SetBits(GPIOA,GPIO_Pin_7);             // Set PA7 as SPI_MOSI and set high
		 else
		 	GPIO_ResetBits(GPIOA,GPIO_Pin_7);           // Set PA7 as SPI_MOSI and set  LOW
		 dat = dat << 1;
	     GPIO_SetBits(GPIOA,GPIO_Pin_5);                // Set PA5 as SPI_CLK and set high
   }
   GPIO_ResetBits(GPIOA,GPIO_Pin_5);                    // Set PA5 as SPI_CLK and set  LOW
}

/*******************************************************************************
* Function    : _recv_byte
* Description : Recieve one Byte
* Arguments   :  			
* Return      :	dat recieved
*******************************************************************************/ 
static u8 _recv_byte()
{
    unsigned char i,dat,temp;
	GPIO_ResetBits(GPIOA,GPIO_Pin_5);                     // Set PA5 as SPI_CLK and set  LOW
	dat =0; temp =0x80;
	for (i=0;i<8;i++)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_5);                    // Set PA5 as SPI_CLK and set high
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)&0x01)   // Set PA6 as SPI_MISO and Read 
			dat|= temp;					    
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);                  // Set PA5 as SPI_CLK and set  LOW
		temp >>= 1;		
	}
	return dat;
}

/*******************************************************************************
* Function    : SPI_SendBuff
* Description : Send  num Byte  Data
* Arguments   : buf  -- send buffer
				num  -- data number 			
* Return      :	
*******************************************************************************/ 
void SPI_SendBuff(unsigned char *buf,unsigned int num)
{
 	if ((buf== NULL)||(num == 0)) return;
	while( num--)
	{
	 	_send_byte(*buf++);
	}  
}	   

/*******************************************************************************
* Function    : SPI_RecvBuff
* Description : Recieve  num Byte  Data
* Arguments   : buf  -- Rec buffer
				num  -- data number 			
* Return      :	
*******************************************************************************/ 
void SPI_RecvBuff(unsigned char *buf,unsigned int num)
{
	if (num == 0) 		
		num= 0x100;
	while(num--)
	{
		*buf++ = _recv_byte();	
	 }
}

/*******************************************************************************
* Function    : SPI_Init_GPIO
* Description : GPIO init  as SPI
* Arguments   :  			
* Return      :
*******************************************************************************/ 
void SPI_Init_GPIO(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	      // Enable GPIOA_CLOCK
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	      // Enable GPIOB_CLOCK
	GPIOA->CRL = 0x38334444;                                        // Set GPIOa 0~7 as IIIIOOIO   (SSN AS OUT  MOSI as OUT  CLK AS OUT   and  MISO AS IN)
	GPIOB->CRL = 0x44444433;                                        // Set 
	GPIOB->CRH = 0x44444443;                                        // Set  
	GPIO_SetBits(GPIOA,GPIO_Pin_4);    							  // Set PA4 as SPI_SSN and set SSN High
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);    						  // Set PB0 as MOD0 and set SSN LOW
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);    						  // Set PB1 as STANDBY and set SSN LOW
}


/*******************************************************************************
* Function    : LED_On
* Description : Set PB8 as output and Low
* Arguments   :  			
* Return      :
*******************************************************************************/ 
void LED_On(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	    // Enable GPIOB_CLOCK	
		GPIOB->CRH = 0x44444433;                                      // Set PB8 as output
		GPIO_ResetBits(GPIOB,GPIO_Pin_8);    						// Set PB8 as LED control and light
}

/*******************************************************************************
* Function    : LED_Off
* Description : Set PB8 as output and High
* Arguments   :  			
* Return      :
*******************************************************************************/ 
void LED_Off(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	     
		GPIOB->CRH = 0x44444433;
		GPIO_SetBits(GPIOB,GPIO_Pin_8);    							  // Set PB8 as LED control and  off
}

/*******************************************************************************
* Function    : Buzzer_On
* Description : Set PB9 as output and High
* Arguments   :  			
* Return      :
*******************************************************************************/
void Buzzer_On(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	      // Enable GPIOB_CLOCK	
		GPIOB->CRH = 0x44444433;                                        // Set PB9 as output
		//GPIO_SetBits(GPIOB,GPIO_Pin_9);   							  // Set PB9 as MOD0 and set SSN LOW
		GPIO_ResetBits(GPIOB,GPIO_Pin_9);    						  // Set PB9 as Buzzer Controland shutdown
}

/*******************************************************************************
* Function    : Buzzer_Off
* Description : Set PB9 as output and Low
* Arguments   :  			
* Return      :
*******************************************************************************/
void Buzzer_Off(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	     	
		GPIOB->CRH = 0x44444433;                                       		
		//GPIO_ResetBits(GPIOB,GPIO_Pin_9);    						  // Set PB9 as Buzzer Controland shutdown
		GPIO_SetBits(GPIOB,GPIO_Pin_9);   							  // Set PB9 as MOD0 and set SSN LOW
}



