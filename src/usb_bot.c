/**
  ******************************************************************************
  * @file    usb_bot.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   BOT State Machine management
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
#include "usb_scsi.h"
#include "hw_config.h"
#include "usb_regs.h"
#include "usb_mem.h"
#include "usb_conf.h"
#include "usb_bot.h"
#include "usb_lib.h"
#include "ccid.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Bot_State;
uint8_t Bulk_Data_Buff[BULK_MAX_PACKET_SIZE];  /* data buffer*/
uint16_t Data_Len;
Bulk_Only_CBW CBW;
Bulk_Only_CSW CSW;
uint32_t SCSI_LBA , SCSI_BlkLen;
extern uint32_t Max_Lun;
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


void usbEpnRxLastAllData(u8 bEpNum,u8* pBufferPointer,u16 sRcvLen)
{
	u16 receivelen;

	while(sRcvLen)
	{
		SetEPRxStatus(bEpNum, EP_RX_VALID);	
		while(!(GetENDPOINT(bEpNum)&0x8000));
		_ClearEP_CTR_RX(bEpNum);
		receivelen = USB_SIL_Read(bEpNum, pBufferPointer);
		pBufferPointer += receivelen;
		sRcvLen -= receivelen;
	}
//	if(receivelen < BULK_MAX_PACKET_SIZE) 
//		return;
}

/*******************************************************************************
* Function Name  : CCID_In
* Description    : CCID IN transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_In (u8 In_Endp,u8 Out_Endp,u8 In_EndpNum,u8 Out_EndpNum)
{
	if(g_sUsbCcidTxLen>BULK_MAX_PACKET_SIZE)	
	{
		USB_SIL_Write(In_Endp, g_abUsbCcidTxBuf+g_sUsbCcidTxTimes*BULK_MAX_PACKET_SIZE, BULK_MAX_PACKET_SIZE);
		SetEPTxCount(In_EndpNum, BULK_MAX_PACKET_SIZE);
    	SetEPTxStatus(In_EndpNum, EP_TX_VALID);
		g_sUsbCcidTxLen -= BULK_MAX_PACKET_SIZE;
		g_sUsbCcidTxTimes ++;	
	}
	else if(g_sUsbCcidTxLen != 0)
	{
		USB_SIL_Write(In_Endp, g_abUsbCcidTxBuf+g_sUsbCcidTxTimes*BULK_MAX_PACKET_SIZE, g_sUsbCcidTxLen);
		SetEPTxCount(In_EndpNum, g_sUsbCcidTxLen);
    	SetEPTxStatus(In_EndpNum, EP_TX_VALID);
		g_sUsbCcidTxLen = 0;
		g_sUsbCcidTxTimes = 0;
	}
		 
	else if((g_sUsbCcidTxnPakageFlag == 0)&&(g_sUsbCcidTxLen == 0))
	{
		USB_SIL_Write(In_Endp, g_abUsbCcidTxBuf+g_sUsbCcidTxTimes*BULK_MAX_PACKET_SIZE, g_sUsbCcidTxLen);
		SetEPTxCount(In_EndpNum, g_sUsbCcidTxLen);
    	SetEPTxStatus(In_EndpNum, EP_TX_VALID);
		g_sUsbCcidTxnPakageFlag = 1;
	}
		
	else
	 if(g_sUsbCcidTxLen == 0)
	{
		SetEPRxStatus(Out_EndpNum, EP_RX_VALID);// enable the Endpoint to receive the next cmd
		if (GetEPRxStatus(Out_Endp) == EP_RX_STALL)
		{
			SetEPRxStatus(Out_Endp, EP_RX_VALID);// enable the Endpoint to receive the next cmd
		}
	}
}

/*******************************************************************************
* Function Name  : CCID_Out
* Description    : CCID OUT transfer.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_Out (u8 In_Endp,u8 Out_Endp,u8 In_EndpNum)
{
	u16	receivelen = 0;

  receivelen = USB_SIL_Read(Out_Endp, g_abUsbCcidRxBuf);

	if(receivelen == 0)
		return ;
	else if(receivelen < BULK_MAX_PACKET_SIZE)
	{
	}
	else
	{
		receivelen = g_abUsbCcidRxBuf[2];	
		receivelen = (receivelen<<8)+g_abUsbCcidRxBuf[1]+10 - BULK_MAX_PACKET_SIZE;
		usbEpnRxLastAllData(Out_Endp,g_abUsbCcidRxBuf+BULK_MAX_PACKET_SIZE,receivelen);	//接收剩余的数据
	}
	g_bUsbCcidRcvDataFlag = 1;

	if(g_bUsbCcidRcvDataFlag)
	{
		if(!st_ccidCmdMessage.bLockCard)									//	ICC is not locked with _Mechanical
		{
			CCID_main();	
		} 
		g_bUsbCcidRcvDataFlag = 0;
		g_sUsbCcidTxTimes = 0;
		g_sUsbCcidTxnPakageFlag = (g_sUsbCcidTxLen%BULK_MAX_PACKET_SIZE);

		if(g_sUsbCcidTxLen>BULK_MAX_PACKET_SIZE)	
		{
			USB_SIL_Write(In_Endp, g_abUsbCcidTxBuf, BULK_MAX_PACKET_SIZE);
			SetEPTxCount(In_EndpNum, BULK_MAX_PACKET_SIZE);
	    	SetEPTxStatus(In_EndpNum, EP_TX_VALID);
			g_sUsbCcidTxLen -= BULK_MAX_PACKET_SIZE;
			g_sUsbCcidTxTimes ++;
		}
		else
		{
			USB_SIL_Write(In_Endp, g_abUsbCcidTxBuf, g_sUsbCcidTxLen);
			SetEPTxCount(In_EndpNum, g_sUsbCcidTxLen);
	    	SetEPTxStatus(In_EndpNum, EP_TX_VALID);
			g_sUsbCcidTxLen = 0;
		}
	}
}


/*******************************************************************************
* Function Name  : Transfer_Data_Request
* Description    : Send the request response to the PC HOST.
* Input          : uint8_t* Data_Address : point to the data to transfer.
*                  uint16_t Data_Length : the number of Bytes to transfer.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Transfer_Data_Request(uint8_t* Data_Pointer, uint16_t Data_Len)
{
  USB_SIL_Write(EP1_IN, Data_Pointer, Data_Len);

    SetEPTxStatus(ENDP1, EP_TX_VALID);
    
  Bot_State = BOT_DATA_IN_LAST;
  CSW.dDataResidue -= Data_Len;
  CSW.bStatus = CSW_CMD_PASSED;
}

/*******************************************************************************
* Function Name  : Set_CSW
* Description    : Set the SCW with the needed fields.
* Input          : uint8_t CSW_Status this filed can be CSW_CMD_PASSED,CSW_CMD_FAILED,
*                  or CSW_PHASE_ERROR.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_CSW (uint8_t CSW_Status, uint8_t Send_Permission)
{
  CSW.dSignature = BOT_CSW_SIGNATURE;
  CSW.bStatus = CSW_Status;

  USB_SIL_Write(EP1_IN, ((uint8_t *)& CSW), CSW_DATA_LENGTH);

  Bot_State = BOT_ERROR;
  if (Send_Permission)
  {
    Bot_State = BOT_CSW_Send;
    SetEPTxStatus(ENDP1, EP_TX_VALID);
  }
}

/*******************************************************************************
* Function Name  : Bot_Abort
* Description    : Stall the needed Endpoint according to the selected direction.
* Input          : Endpoint direction IN, OUT or both directions
* Output         : None.
* Return         : None.
*******************************************************************************/
void Bot_Abort(uint8_t Direction)
{
  switch (Direction)
  {
    case DIR_IN :
      SetEPTxStatus(ENDP1, EP_TX_STALL);
      break;
    case DIR_OUT :
      SetEPRxStatus(ENDP2, EP_RX_STALL);
      break;
    case BOTH_DIR :
      SetEPTxStatus(ENDP1, EP_TX_STALL);
      SetEPRxStatus(ENDP2, EP_RX_STALL);
      break;
    default:
      break;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


