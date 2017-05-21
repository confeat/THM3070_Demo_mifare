/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
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
#include "usb_lib.h"
#include "usb_bot.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    : EP1 IN Callback Routine
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
  CCID_In(EP1_IN,EP2_OUT,ENDP1,ENDP2);
}

void EP4_IN_Callback(void)
{
  CCID_In(EP4_IN,EP5_OUT,ENDP4,ENDP5);
}
/*******************************************************************************
* Function Name  : EP2_OUT_Callback.
* Description    : EP2 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP2_OUT_Callback(void)
{
  CCID_Out(EP1_IN,EP2_OUT,ENDP1);
}
void EP5_OUT_Callback(void)
{
  CCID_Out(EP4_IN,EP5_OUT,ENDP4);
}
void EP3_IN_Callback(void)
{

}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

