/**
  ******************************************************************************
  * @file    usb_prop.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to CCID Demo
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
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_bot.h"
#include "usb_prop.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined (USE_STM3210E_EVAL)
uint32_t Max_Lun = 1;
#else
uint32_t Max_Lun = 0;
#endif

DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    CCID_init,
    CCID_Reset,
    CCID_Status_In,
    CCID_Status_Out,
    CCID_Data_Setup,
    CCID_NoData_Setup,
    CCID_Get_Interface_Setting,
    CCID_GetDeviceDescriptor,
    CCID_GetConfigDescriptor,
    CCID_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };

USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    CCID_GetConfiguration,
    CCID_SetConfiguration,
    CCID_GetInterface,
    CCID_SetInterface,
    CCID_GetStatus,
    CCID_ClearFeature,
    CCID_SetEndPointFeature,
    CCID_SetDeviceFeature,
    CCID_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t*)CCID_DeviceDescriptor,
    CCID_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t*)CCID_ConfigDescriptor,
    CCID_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR String_Descriptor[6] = 
  {
    {(uint8_t*)CCID_StringLangID, CCID_SIZ_STRING_LANGID},
    {(uint8_t*)CCID_StringVendor, CCID_SIZ_STRING_VENDOR},
    {(uint8_t*)CCID_StringProduct, CCID_SIZ_STRING_PRODUCT},
    {(uint8_t*)CCID_StringSerial, CCID_SIZ_STRING_SERIAL},
    {(uint8_t*)CCID_StringInterface, CCID_SIZ_STRING_INTERFACE1},
	{(uint8_t*)CCID_StringInterface2, CCID_SIZ_STRING_INTERFACE2},
  };

/* Extern variables ----------------------------------------------------------*/
extern unsigned char Bot_State;
extern Bulk_Only_CBW CBW;

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : CCID_init
* Description    : CCID init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_init()
{
  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum();

  pInformation->Current_Configuration = 0;

  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : CCID_Reset
* Description    : CCID reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_Reset()
{
  /* Set the device as not configured */
  Device_Info.Current_Configuration = 0;

  /* Current Feature initialization */
  pInformation->Current_Feature = CCID_ConfigDescriptor[7];

  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_NAK);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_BULK);
  SetEPRxAddr(ENDP2, ENDP2_RXADDR);
  SetEPRxCount(ENDP2, Device_Property.MaxPacketSize);
  SetEPRxStatus(ENDP2, EP_RX_VALID);
  SetEPTxStatus(ENDP2, EP_TX_DIS);

   /* Initialize Endpoint 3 */
  SetEPType(ENDP3, EP_INTERRUPT);
  SetEPTxAddr(ENDP3, ENDP3_TXADDR);
//  SetEPTxCount(ENDP3,2);/////////////////////////////////
  SetEPTxStatus(ENDP3, EP_TX_NAK);
  SetEPRxStatus(ENDP3, EP_RX_DIS);

    /* Initialize Endpoint 4 */
  SetEPType(ENDP4, EP_BULK);
  SetEPTxAddr(ENDP4, ENDP4_TXADDR);
  SetEPTxStatus(ENDP4, EP_TX_NAK);
  SetEPRxStatus(ENDP4, EP_RX_DIS);

    /* Initialize Endpoint 5 */
  SetEPType(ENDP5, EP_BULK);
  SetEPRxAddr(ENDP5, ENDP5_RXADDR);
  SetEPRxCount(ENDP5, Device_Property.MaxPacketSize);
  SetEPRxStatus(ENDP5, EP_RX_VALID);
  SetEPTxStatus(ENDP5, EP_TX_DIS);

  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Set the device to response on default address */
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;

  CBW.dSignature = BOT_CBW_SIGNATURE;
  Bot_State = BOT_IDLE;

}

/*******************************************************************************
* Function Name  : CCID_SetConfiguration
* Description    : Handle the SetConfiguration request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_SetConfiguration(void)
{
  if (pInformation->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
   
    ClearDTOG_TX(ENDP1);
    ClearDTOG_RX(ENDP2);

    Bot_State = BOT_IDLE; /* set the Bot state machine to the IDLE state */
  }
}

/*******************************************************************************
* Function Name  : CCID_ClearFeature
* Description    : Handle the ClearFeature request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_ClearFeature(void)
{
  /* when the host send a CBW with invalid signature or invalid length the two
     Endpoints (IN & OUT) shall stall until receiving a CCID Reset     */
  if (CBW.dSignature != BOT_CBW_SIGNATURE)
    Bot_Abort(BOTH_DIR);
}

/*******************************************************************************
* Function Name  : CCID_SetConfiguration.
* Description    : Update the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : CCID_Status_In
* Description    : CCID Status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_Status_In(void)
{
  return;
}

/*******************************************************************************
* Function Name  : CCID_Status_Out
* Description    : CCID Status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CCID_Status_Out(void)
{
  return;
}

/*******************************************************************************
* Function Name  : CCID_Data_Setup.
* Description    : Handle the data class specific requests..
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT CCID_Data_Setup(uint8_t RequestNo)
{
  uint8_t    *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == GET_MAX_LUN) && (pInformation->USBwValue == 0)
      && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x01))
  {
    CopyRoutine = Get_Max_Lun;
  }
  else
  {
    return USB_UNSUPPORT;
  }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);

  return USB_SUCCESS;

}

/*******************************************************************************
* Function Name  : CCID_NoData_Setup.
* Description    : Handle the no data class specific requests.
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT CCID_NoData_Setup(uint8_t RequestNo)
{
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == CCID_RESET) && (pInformation->USBwValue == 0)
      && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00))
  {
    /* Initialize Endpoint 1 */
    ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 2 */
    ClearDTOG_RX(ENDP2);

    /*initialize the CBW signature to enable the clear feature*/
    CBW.dSignature = BOT_CBW_SIGNATURE;
    Bot_State = BOT_IDLE;

    return USB_SUCCESS;
  }
  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : CCID_Get_Interface_Setting
* Description    : Test the interface and the alternate setting according to the
*                  supported one.
* Input          : uint8_t Interface, uint8_t AlternateSetting.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT CCID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;/* in this application we don't have AlternateSetting*/
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;/*in this application we have only 1 interfaces*/
  }
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : CCID_GetDeviceDescriptor
* Description    : Get the device descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *CCID_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor );
}

/*******************************************************************************
* Function Name  : CCID_GetConfigDescriptor
* Description    : Get the configuration descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *CCID_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor );
}

/*******************************************************************************
* Function Name  : CCID_GetStringDescriptor
* Description    : Get the string descriptors according to the needed index.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *CCID_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;

  if (wValue0 > 6)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

/*******************************************************************************
* Function Name  : Get_Max_Lun
* Description    : Handle the Get Max Lun request.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *Get_Max_Lun(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
    return 0;
  }
  else
  {
    return((uint8_t*)(&Max_Lun));
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
