/**
  ******************************************************************************
  * @file    usb_prop.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to CCID Demo (Endpoint 0)
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PROP_H
#define __USB_PROP_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define CCID_GetConfiguration          NOP_Process
/* #define CCID_SetConfiguration          NOP_Process*/
#define CCID_GetInterface              NOP_Process
#define CCID_SetInterface              NOP_Process
#define CCID_GetStatus                 NOP_Process
/* #define CCID_ClearFeature              NOP_Process*/
#define CCID_SetEndPointFeature        NOP_Process
#define CCID_SetDeviceFeature          NOP_Process
/*#define CCID_SetDeviceAddress          NOP_Process*/

/* CCID Requests*/
#define GET_MAX_LUN                0xFE
#define CCID_RESET         		   0xFF
#define LUN_DATA_LENGTH            1

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void CCID_init(void);
void CCID_Reset(void);
void CCID_SetConfiguration(void);
void CCID_ClearFeature(void);
void CCID_SetDeviceAddress (void);
void CCID_Status_In (void);
void CCID_Status_Out (void);
RESULT CCID_Data_Setup(uint8_t);
RESULT CCID_NoData_Setup(uint8_t);
RESULT CCID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *CCID_GetDeviceDescriptor(uint16_t );
uint8_t *CCID_GetConfigDescriptor(uint16_t);
uint8_t *CCID_GetStringDescriptor(uint16_t);
uint8_t *Get_Max_Lun(uint16_t Length);


#endif /* __USB_PROP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
