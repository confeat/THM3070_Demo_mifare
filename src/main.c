/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   CCID demo main file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h" 
#include "hw_config.h" 
#include "usb_lib.h"
#include "usb_pwr.h"
#include "ccid.h" 
#include "thm3070.h"
#include "spi.h"
#include "iso14443.h"

extern uint16_t MAL_Init (uint8_t lun);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/


u8 RDR_to_PC_NotifySlotChang[2];
u8 mode_flag = 1;

void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //开启辅助时钟	  
	AFIO->MAPR&=0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR|=temp;       //设置jtag模式
} 

int main(void)
{
	JTAG_Set(1);
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    THM_Reset(); 		//Initial  GPIO and config THM3070
    delay_ms(1);			            
    ISO14443_carrier_on(); 
		LED_Off();
		Buzzer_Off();   

    while (bDeviceState != CONFIGURED);


		RDR_to_PC_NotifySlotChang[0]= 0x50;
		RDR_to_PC_NotifySlotChang[1]= 0x01;	

    while (1)
    {

		if(mode_flag == 1)
		{
	  		if(bCardInserted == 0)
			{
				if(((__FindCard())==0x01) && (RDR_to_PC_NotifySlotChang[1]!=0x03))
				{
					RDR_to_PC_NotifySlotChang[1] = 0x03;	//	ICC is not changed
					USB_SIL_Write(EP3_IN,((uint8_t *)RDR_to_PC_NotifySlotChang),2);
					SetEPTxStatus(ENDP3, EP_TX_VALID);
					bCardInserted = 1;
					apdusendtime =0;
					LED_On();
					Buzzer_On();
					delay_ms(1000);
					Buzzer_Off();
				}
			}
			
	
			__disable_irq();
	  	    if(((__CardAbsent())==0x00) && (RDR_to_PC_NotifySlotChang[1]!=0x02))
			{
				RDR_to_PC_NotifySlotChang[1] = 0x02;	//	ICC is changed
				USB_SIL_Write(EP3_IN,((uint8_t *)RDR_to_PC_NotifySlotChang),2);
				SetEPTxStatus(ENDP3, EP_TX_VALID);
				bCardInserted = 0;
				LED_Off();
			}
			__enable_irq();		
		}		
		delay_ms(50);	  
    }			
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
