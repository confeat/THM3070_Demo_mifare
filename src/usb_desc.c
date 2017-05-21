/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptors for CCID Device
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
#include "usb_desc.h"

const uint8_t CCID_DeviceDescriptor[CCID_SIZ_DEVICE_DESC] =
  {
    0x12,			//	length
    0x01,			//	Fixed device 
    0x10,0x01,		//	USB 1.1
    0x00,
    0x00,
    0x00,
    0x40,		//	Default max ep0 packet length 
    0xff,0xff,		//	VID
    0xff,0xf5,		//	PID
    0x00,0x01,		//	Device release number
    0x01,			//	Index of string descriptor describing manufacturer
    0x02,			//	Index of string descriptor describing product
    0x03,			//	Index of string descriptor describing the device¡¯s serial number
    0x01,			//	Number of possible configurations
  };
const uint8_t CCID_ConfigDescriptor[CCID_SIZ_CONFIG_DESC] =
  {

	0x09,			//	length
    0x02,			//	Fixed config 
    0xAA,0x00, 		//	0x6D Bytes in total
    0x02,			//	support 2 interface
    0x01,			//	Configuration Value is 1
    0x00,			//	Index of string descriptor describing this configuration
    0x80,			//	Doesn't support remote wake up        
    0xFA,			//	100mA

//ccid1
    0x09, 					//	length
    0x04,					//	Fixed interface	
    0x00,					//	Interface  Number is 0
    0x00,					//	Alternate Setting is 0
    0x03,					//	Ep number is 3,they are 1,2,3
    0x0B,					//	Smart Card Device Class
    0x00,					//	Subclass code
    0x00,					//	CCID Protocol
    0x04,					//	Index of string descriptor describing this interface

	0x36,		   			//	length
    0x21,		   			//	Functional S/C Device Class descriptor type
    0x00,0x01,	   			//	CCID1.0(CCID1.1)
    0x00,		   			//	1 Slot numbered 0x00
    0x07,		   			//	01H-5.0V,02H-3.0V,03H-1.8V
    0x03,0x00,0x00,0x00,	//	Little Endian,Upper u16 is RFU = 0x0000,Lower u16:0001H-T=0,0002H-T=1
    0xa0,0x0f,0x00,0x00,	//	3.58MHz=3580KHz=0x0DFC
    0xa0,0x0f,0x00,0x00,	//	14.32MHz
    0x00,
    0x00,0x2a,0x00,0x00,	//	9600bps
    0x16,0x40,0x05,0x00,	//	115200bps
    0x00,
    0xfe,0x00,0x00,0x00,	//	IFSD (T=1)
    0x00,0x00,0x00,0x00,	//	Upper u16 is RFU = 0x0000,Lower u8 encodes the supported protocol type
    0x00,0x00,0x00,0x00,	//	No spefcial characteristics
    0x80,0x00,0x04,0x00,
    //0x00,0x00,0x00,0x00,
    0x0f,0x01,0x00,0x00,	//	271B = 10B Header + 5B CMD + 256B data
    0x00,
    0x00,
    0x00,0x00,				//	No LCD
    0x00,
    0x01, 					//	1 slot can be simultaneously busy

//	0x15,						//	total length
//                            //	ep2 setting
    0x07,
    0x05, 					//	Fixed endpoint
    0x81,					//	IN,Ep1
    0x02,					//	BULK
    0x40,0x00,		//	max packet length is BULKPktSize
    0x00,	   				//	Don't care
						//	ep2 setting
    0x07,
    0x05,					//	Fixed endpoint
    0x02,					//	OUT Ep2
    0x02,					//	BULK
    0x40,0x00,		//	max packet length is BULKPktSize
    0x00,					//	Don't care	
    
    0x07,
    0x05,
    0x83,
    0x03,
    0x08,0x00,
    0xFF,
    /*32*/

//	ccid2
	0x09, 					//	length
    0x04,					//	Fixed interface	
    0x01,					//	Interface  Number is 1
    0x00,					//	Alternate Setting is 0
    0x02,					//	Ep number is 2,they are 4,5
    0x0B,					//	Smart Card Device Class
    0x00,					//	Subclass code
    0x00,					//	CCID Protocol
    0x05,					//	Index of string descriptor describing this interface

	0x36,		   			//	length
    0x21,		   			//	Functional S/C Device Class descriptor type
    0x00,0x01,	   			//	CCID1.0(CCID1.1)
    0x00,		   			//	1 Slot numbered 0x00
    0x07,		   			//	01H-5.0V,02H-3.0V,03H-1.8V
    0x03,0x00,0x00,0x00,	//	Little Endian,Upper u16 is RFU = 0x0000,Lower u16:0001H-T=0,0002H-T=1
    0xa0,0x0f,0x00,0x00,	//	3.58MHz=3580KHz=0x0DFC
    0xa0,0x0f,0x00,0x00,	//	14.32MHz
    0x00,
    0x00,0x2a,0x00,0x00,	//	9600bps
    0x16,0x40,0x05,0x00,	//	115200bps
    0x00,
    0xfe,0x00,0x00,0x00,	//	IFSD (T=1)
    0x00,0x00,0x00,0x00,	//	Upper u16 is RFU = 0x0000,Lower u8 encodes the supported protocol type
    0x00,0x00,0x00,0x00,	//	No spefcial characteristics
    0x80,0x00,0x04,0x00,
    //0x00,0x00,0x00,0x00,
    0x0f,0x01,0x00,0x00,	//	271B = 10B Header + 5B CMD + 256B data
    0x00,
    0x00,
    0x00,0x00,				//	No LCD
    0x00,
    0x01, 					//	1 slot can be simultaneously busy

//
//	 	0x36,		   			//	length
//    0x21,		   			//	Functional S/C Device Class descriptor type
//    0x10,0x01,	   			//	CCID1.0(CCID1.1)
//    0x00,		   			//	1 Slot numbered 0x00
//    0x01,		   			//	01H-5.0V,02H-3.0V,03H-1.8V
//    0x03,0x00,0x00,0x00,	//	Little Endian,Upper u16 is RFU = 0x0000,Lower u16:0001H-T=0,0002H-T=1
//    0xfc,0x0d,0x00,0x00,	//	3.58MHz=3580KHz=0x0DFC
//    0xf0,0x37,0x00,0x00,	//	14.32MHz
//    0x00,
//    0x80,0x25,0x00,0x00,	//	9600bps
//    0x00,0xc2,0x01,0x00,	//	115200bps
//    0x00,
//    0x10,0x10,0x00,0x00,	//	IFSD (T=1)
//    0x00,0x00,0x00,0x00,	//	Upper u16 is RFU = 0x0000,Lower u8 encodes the supported protocol type
//    0x00,0x00,0x00,0x00,	//	No spefcial characteristics
//    0x72,0x06,0x04,0x00,
//    //0x00,0x00,0x00,0x00,
//    0x1e,0x04,0x00,0x00,	//	271B = 10B Header + 5B CMD + 256B data
//    0xff,
//    0xff,
//    0x00,0x00,				//	No LCD
//    0x00,
//    0x01, 					//	1 slot can be simultaneously busy


//	ep4 setting
    0x07,
    0x05, 					//	Fixed endpoint
    0x84,					//	IN,Ep4
    0x02,					//	BULK
    0x40,0x00,				//	max packet length is BULKPktSize
    0x00,	   				//	Don't care
//	ep5 setting
    0x07,
    0x05,					//	Fixed endpoint
    0x05,					//	OUT Ep5
    0x02,					//	BULK
    0x40,0x00,				//	max packet length is BULKPktSize
    0x00					//	Don't care	
  };
const uint8_t CCID_StringLangID[CCID_SIZ_STRING_LANGID] =
  {
    CCID_SIZ_STRING_LANGID,
    0x03,
    0x09,
    0x04
  }
  ;      /* LangID = 0x0409: U.S. English */
const uint8_t CCID_StringVendor[CCID_SIZ_STRING_VENDOR] =
  {
    CCID_SIZ_STRING_VENDOR, /* Size of manufacturer string */
    0x03,           /* bDescriptorType = String descriptor */
    /* Manufacturer: "STMicroelectronics" */
    'T', 0, 'M', 0, 'C', 0
  };
const uint8_t CCID_StringProduct[CCID_SIZ_STRING_PRODUCT] =
  {
    CCID_SIZ_STRING_PRODUCT,
    0x03,
    /* Product name: "STM32F10x:USB CCID" */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0, 'U', 0, 'S', 0, 'B', 0,
    ' ', 0, ' ', 0, 'C', 0, 'C', 0, 'I', 0, 'D', 0

  };

uint8_t CCID_StringSerial[CCID_SIZ_STRING_SERIAL] =
  {
    CCID_SIZ_STRING_SERIAL,
    0x03,
    /* Serial number*/
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0
  };
const uint8_t CCID_StringInterface[CCID_SIZ_STRING_INTERFACE1] =
  {
    CCID_SIZ_STRING_INTERFACE1,
    0x03,
    /* Interface 0: "ST CCID1" */
    'C', 0, 'o', 0, 'n', 0, 't', 0, 'a', 0, 'c', 0, 't', 0,	'l', 0, 'e', 0, 's', 0, 's', 0, ' ', 0, 'R', 0, 'e', 0,	'a' , 0, 'd', 0, 'e', 0,	'r' , 0
  };

  const uint8_t CCID_StringInterface2[CCID_SIZ_STRING_INTERFACE2] =
  {
    CCID_SIZ_STRING_INTERFACE2,
    0x03,
    /* Interface 0: "ST CCID2" */
    'C', 0, 'o', 0, 'n', 0, 't', 0, 'a', 0, 'c', 0, 't', 0,	'l', 0, 'e', 0, 's', 0, 's', 0, ' ', 0, 'R', 0, 'e', 0,	'a' , 0, 'd', 0, 'e', 0,	'r' , 0, '_', 0,	'T' , 0
  };
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
