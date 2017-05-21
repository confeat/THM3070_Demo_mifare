/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptor Header for CCID Device
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
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define CCID_SIZ_DEVICE_DESC              18
#define CCID_SIZ_CONFIG_DESC              0xAA

#define CCID_SIZ_STRING_LANGID            4
#define CCID_SIZ_STRING_VENDOR            8
#define CCID_SIZ_STRING_PRODUCT           32
#define CCID_SIZ_STRING_SERIAL            26
#define CCID_SIZ_STRING_INTERFACE1         38
#define CCID_SIZ_STRING_INTERFACE2         42


/* Exported functions ------------------------------------------------------- */
extern const uint8_t CCID_DeviceDescriptor[CCID_SIZ_DEVICE_DESC];
extern const uint8_t CCID_ConfigDescriptor[CCID_SIZ_CONFIG_DESC];

extern const uint8_t CCID_StringLangID[CCID_SIZ_STRING_LANGID];
extern const uint8_t CCID_StringVendor[CCID_SIZ_STRING_VENDOR];
extern const uint8_t CCID_StringProduct[CCID_SIZ_STRING_PRODUCT];
extern uint8_t CCID_StringSerial[CCID_SIZ_STRING_SERIAL];
extern const uint8_t CCID_StringInterface[CCID_SIZ_STRING_INTERFACE1];
extern const uint8_t CCID_StringInterface2[CCID_SIZ_STRING_INTERFACE2];

#endif /* __USB_DESC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


