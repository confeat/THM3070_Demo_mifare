/********************************************************************************
* Copyright (c) 2012, Beijing Tongfang Microelectroics Co., Ltd.
* All rights reserved.
* Module: ISO14443.c
* Author: Cui Lu
* Version: V1.0
* History:
*   2013-05-30 Original version
********************************************************************************/

#include "ISO14443.h"
#include "thm3070.h"
#include "string.h"
#include "SPI.h"
#include "EMV_Main.h"

struct ST_PICC mypicc[MAXSLOT];
//set global variables
u8   c_WUPA = 0x52;
u8  slotMarker = 0x05;
u8  c_WUPB[] = {0x05,0x00,0x08};
u8  c_SELECT[] = {0x93,0x70};
u8  c_RATS[] = {0xE0,(FSDI<<4)};
u8  c_HLTA[]={0x50,0x00};
u16 FSD[] = {16,24,32,40,48,64,96,128,256,256};
u8  Slot_marker[] = {0x05,0x15};
u8  PPS_TEMP[];

u8  sendtime = 0;
u8  fwtpoll=0x5; //used to adjust tp time for wupa and wupb
u8  SendBaud=SND_BAUD_106K;
u8  RevBaud=RCV_BAUD_106K;
u8  HandleCollision = 0;

/*******************************************************************************
* Function    : THM_En_2Tx
* Description : THM3070 RF initial and open	
* Arguments   : 			
* Return      :	
*******************************************************************************/ 
void THM_En_2Tx()
{
	THM_WriteReg(DP1,0xc0);
	THM_WriteReg(DP0,0xC0);
	THM_WriteReg(DN1,0x60);
	THM_WriteReg(DN0,0x17);
	THM_WriteReg(CTR_RX,0x40); 	          // Enable Digtal_TST and set 
	THM_WriteReg(EGT,0x01); 	          //
	
}

/*******************************************************************************
* Function    : THM_DIs_2Tx
* Description : THM3070 RF closed	
* Arguments   : 			
* Return      :	
*******************************************************************************/
void THM_DIs_2Tx()
{
	THM_WriteReg(CONTROL_TX,0x73);
}

/*******************************************************************************
* Function    : ISO14443_carrier_on
* Description : carrier_on	
* Arguments   : 			
* Return      :	
*******************************************************************************/
void ISO14443_carrier_on(void)
{ 
//	THM_Reset();		            //Initial  GPIO and config THM3070
	THM_WriteReg(CONTROL_TX,0x72);	//Open RF
	THM_EnableEMV();                //enable emv error
}

/*******************************************************************************
* Function    : ISO14443_carrier_off
* Description : carrier_off	
* Arguments   : 			
* Return      :	
*******************************************************************************/
void ISO14443_carrier_off()
{ 	
//	THM_Reset();	  //Initial  GPIO as SPI		
	THM_DIs_2Tx();	  //Close RF	
}

/*******************************************************************************
* Function    : ISO14443_carrier
* Description : carrier open  or off
* Arguments   : Type =0   carrier off
				Type =1   carrier on  and set protocol as Type A  106k
				else      carrier on  and set protocol as Type B  106k 
* Return      : None.
*******************************************************************************/
void ISO14443_carrier(u8 Type)
{
	if( 0 == Type)
		ISO14443_carrier_off();
	else
	{
		ISO14443_carrier_on();
		if( 1 == Type)
			THM_ChangeProtBaud(TYPE_A,SND_BAUD_106K,RCV_BAUD_106K);
		else 	
		    THM_ChangeProtBaud(TYPE_B,SND_BAUD_106K,RCV_BAUD_106K);
	}	
}
/*******************************************************************************
* Function    : ISO14443_reset
* Description : carrier off, delay  t_reset(ms)	and then carrier open
* Arguments   : t_reset--time between  carrier off and On
* Return      : None.
*******************************************************************************/
void ISO14443_reset(u8 t_reset)
{	
	THM_Reset();					   //reset THM
	THM_WriteReg(CONTROL_TX,0x73);	   //Close RF
	delay_ms(t_reset);				   //waite treset
	THM_WriteReg(CONTROL_TX,0x72);	   //open RF 
}

/*******************************************************************************
* Function    : ISO14443_WUPA
* Description : send WUPA command, and receive ATQA
* Arguments   : ATQA--ATQA of receive			
* Return      : len---Length of ATQA
				Other-Error	
*******************************************************************************/
s16	ISO14443_WUPA(u8 *ATQA)
{
	u16 len;
	u8 rvsta;
	u8 i;
	u8 check=0;		//u8 ck_ATQA=ATQA[0];
	
	THM_SetFWT(fwtpoll);

	//set typeA
	THM_WriteReg(CONTROL_TX,0x72);		//FORCE 100%module  add by lichong
	delay_ms(1);
	THM_ChangeProtBaud(TYPE_A,SND_BAUD_106K,RCV_BAUD_106K);
	THM_SendFrame(&c_WUPA,1);			//send WUPA
	rvsta=THM_WaitReadFrame(&len,ATQA);	//len=2;
	//return error code
	if((rvsta&FEND)&&(len == 2)){					  
	    for(i=0;i<5;i++){
			check+=(ATQA[0]>>i)&0x01;
		}
		 if(((ATQA[0]&0xC0) == 0xC0)||(check != 1)){	   
			return ERR_REV_WUPA;
		}else
      		return len;
	}
	else if(rvsta&CERR)
		return ERR_REV_WUPA;
	else if(rvsta&PERR)
		return ERR_REV_WUPA;
	else if(rvsta&TMROVER)
		return ERR_TIMEOUT_WUPA;                   
	else
		return ERR_REV_WUPA;	
}

/*******************************************************************************
* Function    : ISO14443_Anticollision_TypeA
* Description : anticollision and select for Type A
* Arguments   : rbuffer--ATS of receive			
* Return      : len---Length of ATS
				Other-Error	
*******************************************************************************/
s16	ISO14443_Anticollision_TypeA(u8 *rbuffer)
{
	u16 iLen;
	s16 R_sta;
	u8 i,j;
	u8 SAK;
	u8 RF_buf[7];
	u8 ATQA[10];
	u8 caslevel = 0x93;
	s16 rvsta; 

	rvsta=ISO14443_WUPA(ATQA);				//send WUPA
	
	//If timeout, resend WUPA twice
	for(j=0;j<MAX_RETRANS_WUPAB;j++){
		if(rvsta==ERR_TIMEOUT_WUPA)
			rvsta=ISO14443_WUPA(ATQA);
	} 
	if(rvsta<0)	
	{
		return rvsta;
	}
	//The process of anticollision
	for (i=0;i<3;i++)
	{  
		delay_ms(fdtpcd);               
		//Anticollision of caslevel
		R_sta = sendAC(caslevel, RF_buf+2); 				//Return UID

		if(R_sta==ERR_RECEIVE_TIMEOVER)						//if timeout, resent comand
			R_sta = sendAC(caslevel, RF_buf+2);
		if (R_sta <0 )
			return R_sta;
		else if(((ATQA[0]&0xC0) == 0x00)&&(RF_buf[2] == 0x88))  //single UID	and first UID byte=88
			return ERR_REV_AC;
		else{		
			memcpy((rbuffer+5*i),RF_buf+2,5);			    //copy UID to rbuffer
		}	
		// read SAK
		RF_buf[0] = caslevel;
		RF_buf[1] = 0X70;
		delay_ms(fdtpcd);               
		THM_SendFrame(RF_buf,7);
				 		
		R_sta = THM_WaitReadFrame(&iLen, &SAK);	 			//level = THM_ReadReg(CRCSEL);
	    
		if(R_sta==TMROVER)
		{						 							//If timeout, resent Select command
			THM_SendFrame(RF_buf,7);		 
			R_sta = THM_WaitReadFrame(&iLen, &SAK);
		}
		//check SAK
		if ((R_sta != FEND)||(iLen!=1)||((ATQA[0]&0xC0) == 0x00)&&((SAK&0x04) != 0))		
			return ERR_RECEIVE_OTHER;
		else if ((SAK & 0x04) != 0)
		{
			caslevel += 2;				  		 			//Next Level
		}
		else		
			break; 
	}//end of for loop
	return 5*(i+1); 
}

/*******************************************************************************
* Function    : ISO14443_Anticollision_TypeA_Analog
* Description : anticollision and select for Type A  when EGT  test
* Arguments   : rbuffer--ATS of receive			
* Return      : len---Length of ATS
				Other-Error	
*******************************************************************************/
s16	ISO14443_Anticollision_TypeA_Analog(u8 *rbuffer)
{
	u16 iLen;
	s16 R_sta;
	u8 i;
	u8 SAK;
	u8 RF_buf[7];
	u8 ATQA[10];
	u8 caslevel = 0x93;
	//The process of anticollision
	for (i=0;i<3;i++)
	{  
		delay_ms(fdtpcd);            
		//Anticollision of caslevel
		R_sta = sendAC(caslevel, RF_buf+2); 					//Return UID

		if(R_sta==ERR_RECEIVE_TIMEOVER)							//if timeout, resent comand
		{
			R_sta = sendAC(caslevel, RF_buf+2);
				if(R_sta==ERR_RECEIVE_TIMEOVER)					//if timeout, resent comand
					R_sta = sendAC(caslevel, RF_buf+2);
		}
		if (R_sta <0 )
			return R_sta;
		else if(((ATQA[0]&0xC0) == 0x00)&&(RF_buf[2] == 0x88)) 		//single UID	and first UID byte=88
			return ERR_REV_AC;
		else
		{		
			memcpy((rbuffer+5*i),RF_buf+2,5);					//copy UID to rbuffer
		}	
			
		// read SAK
		RF_buf[0] = caslevel;
		RF_buf[1] = 0X70;
		delay_ms(fdtpcd);                 
		THM_SendFrame(RF_buf,7);		 
		R_sta = THM_WaitReadFrame(&iLen, &SAK);
	    //If timeout, resent Select command
		if(R_sta==TMROVER)
		{
			THM_SendFrame(RF_buf,7);		 
			R_sta = THM_WaitReadFrame(&iLen, &SAK);
		}
		//check SAK
		if ((R_sta != FEND)||(iLen!=1)||((ATQA[0]&0xC0) == 0x00)&&((SAK&0x04) !=0 ))		
			return ERR_RECEIVE_OTHER;
		else if ((SAK & 0x04) != 0)
		{
			caslevel += 2;								   //Next Level
		}
		else		
			break; 
	}//end of for loop
	return 5*(i+1); 
}

/*******************************************************************************
* Function    : ISO14443_RATS
* Description : send RATS and receive ATS, set parameter accordingly
* Arguments   : rbuffer--ATS of receive			
* Return      : len---Length of ATS
				Other-Error	
*******************************************************************************/
s16	ISO14443_RATS(u8 mycid,u8 *ATS)
{
	u16 ATSlen;
	u8 rvsta;
	u8 TL,T0,minATS;
	c_RATS[1]= c_RATS[1]|mycid; 
	
	THM_SetFWT(1<<DFWI);					//set Default FWI

	//send RATS request
	THM_SendFrame(c_RATS,2); 				// Send to Card    
	rvsta=THM_WaitReadFrame(&ATSlen,ATS); 
	//if timeout,or receive noise resent RATS once, then check validty of ATS
	//definition of noise: less than 4bytes with CRC or parity error, FERR, CERR
	if ((rvsta==TMROVER)||(rvsta&FERR)||(rvsta&CERR)||((ATSlen<4)&&(rvsta&PERR))||((ATSlen<4)&&(rvsta&CRCERR))){
		THM_SendFrame(c_RATS,2); // Send to Card      
		rvsta=THM_WaitReadFrame(&ATSlen,ATS); 
	}
	//receive ATS
	if ((rvsta==FEND)&&(ATSlen >= 1))
	{  		
		TL=ATS[0];
		if(ATSlen>1){
			T0=ATS[1];
			minATS=2+((T0>>4)&0x01)+((T0>>5)&0x01)+((T0>>6)&0x01);
		}
		else minATS=1;
		//check the length of ATS
		if((TL!=ATSlen)||(ATSlen<minATS))
			return ERR_REV_ATS;	
		//initialize mypicc parameters
		initPICC(mycid);

		//set PICC parameter accordingly
		mypicc[mycid].TYPE=TYPE_A;
		
		//if T0 exists
		if(TL>1){
				if((T0&0x0F) <= 8)
					mypicc[mycid].FSCI=(T0&0x0F);
				else
					mypicc[mycid].FSCI=8;
		}
		
		
		if((TL>1)&&((T0&0x10) == 0x10))				 // if TA1 exists
			mypicc[mycid].DSDR=ATS[2];	
	
		if((TL>1)&&((T0&0x20) == 0x20))				 // if TB1 exists
		{
			if((ATS[3]&0xf0) == 0xf0)	
				mypicc[mycid].FWI=0x04; 			 // if FWI = F  then FWI = 4
			else
				mypicc[mycid].FWI=ATS[3]>>4;
			    
			if((ATS[3]&0x0F) == 0x0f)						   
			{	
				mypicc[mycid].SFGI = 0;					
			}				    
			else
			{
				mypicc[mycid].SFGI=(ATS[3]&0x0F);
			}
		}
		else
		{
			mypicc[mycid].SFGI = 0;
		}

		mypicc[mycid].blocknumber|= (mypicc[mycid].withCID<<3);
		 	
		return ATSlen;
	} 	
	else 
		return ERR_REV_ATS;
}

/*******************************************************************************
* Function    : ISO14443_sendAPDU
* Description : send APDU command and wait to receive PICC response
* Arguments   : tbuffer --- sendbuffer
				length  ---	APDU length
				rbuffer ---	recieve buffer			
* Return      : len---Length of PICC response
				Other-Error	
*******************************************************************************/
s16	ISO14443_sendAPDU(u8 cid,u8 *tbuffer,u16 length,u8 *rbuffer)
{
	//u8 tempbuffer[300];	//to receive data
	u8 sPCB=mypicc[cid].blocknumber;
	u16 rLEN;
	u8 rPCB;
	u8 rCID;
	u16 SegLength=length;	
	s16 rvstat;
	u8 countNAK = 0;//count NAK times
	u8 retrans = 0;//count retransmission time
	u16 RapduLen = 0;
	u8 ICCchaining = 0;
	u8 PCDchaining = 0;
	u16 PICC_FSC=FSD[mypicc[cid].FSCI];

	//u16 PICC_FSC=16;
	u16 TPDULen=PICC_FSC-3-mypicc[cid].withCID;//minus the length of PCB CID and CRC

	//if PCD chaining
	if(length>TPDULen)
	{ 
			sPCB |= 0x10;//set chaining bit
    		SegLength=TPDULen; 
    		length -= SegLength; 
    		PCDchaining=1;
	}
	
	//send I block
	sendBlock(sPCB,cid,tbuffer,SegLength); 	
	mypicc[cid].blocknumber^=0x01;	//toggle block number 
	sendtime +=1;
	while(1) 
	{	 
		rvstat=recvBlock(&rPCB,&rCID,&rLEN,rbuffer+RapduLen);
		delay_ms(fdtpcd);            
		mypicc[cid].WTXM=1;	//reset WTXM index 

		if(rvstat>=0)  
		{ 			 
			if(!(rPCB&0x02))
				return ERR_RECEIVE_SYNTAX;	
			//if received block is an I block
			if((rPCB&0xC0) == 0x00){ 
				//If b2=0 or b3=1 or b4=1 or wrong block# received length>FSD,or ICCchaining return error code
				if((rPCB&0x04)||(rPCB&0x08)||((rPCB&0x03)==(mypicc[cid].blocknumber&0x03))||(rLEN>FSD[FSDI])||PCDchaining)
					return ERR_RECEIVE_SYNTAX;				
				else if((rPCB&0xF0) == 0x00)//I block & no chaining
				{
					ICCchaining=0;
		    		RapduLen+=rLEN;
					return RapduLen;		
				}
				else if((rPCB&0xF0) == 0x10) //I block with PICC chaining
				{
					countNAK=0;
					ICCchaining=1;
					RapduLen+=rLEN;
					//send R_ACK
					sendBlock((mypicc[cid].blocknumber|0xA0),cid,tbuffer,0);
					mypicc[cid].blocknumber^=0x01;	//toggle block number
					countNAK+=1;
				}
				else
					return ERR_RECEIVE_SYNTAX;
			}			
			//if received block is a RACK block	
			else if((rPCB&0xF0) == 0xA0)
			{  	
				//IF received b4=1 or b3=1, return error code
				if((rPCB&0x04)||(rPCB&0x08))
					return ERR_RECEIVE_SYNTAX;
				//if block number equals to current block number, chaining continued.
				if(((rPCB&0x03)==(mypicc[cid].blocknumber^0x01))&PCDchaining){
					tbuffer+= SegLength;
	   		 		//if terminal chaining       		 		
					if(length>TPDULen)
					{
	        			sPCB=(mypicc[cid].blocknumber|0x10);//set chaining bit
						//sPCB|=0x10;
	    				SegLength=TPDULen; //minus the length of PCB CID and CRC
	        		    length -= SegLength;
	    				PCDchaining=1;
	        		}
	        		else{
	        			sPCB=mypicc[cid].blocknumber;  //block#=02 or 03
	        			SegLength=length;
	        			PCDchaining=0;
	        		}
	   		 		// Send I block.  Then, we can wait to receive.    
					sendBlock(sPCB,cid,tbuffer,SegLength);						
					mypicc[cid].blocknumber^=0x01;	//toggle block number
				}
				else if((rPCB&0x03)==mypicc[cid].blocknumber){
					//retransmit last block
					if(retrans<MAX_RETRANS_APDU){
						sendBlock(sPCB,cid,tbuffer,SegLength);
						retrans+=1;
						countNAK = 0;	//added by wq
					}
					else
					  	return ERR_RECEIVE_SYNTAX;
				}
				else
					return ERR_RECEIVE_SYNTAX;				
			}  
			//if received block is a S(WTX) block
			else if((rPCB&0xF0) == 0xF0)	 // TA 404  
			{
				if(rbuffer[RapduLen]==0)
					return ERR_RECEIVE_SYNTAX;

				if(rbuffer[RapduLen] <= 59)	  
					mypicc[cid].WTXM=rbuffer[RapduLen];
				else
					return ERR_RECEIVE_SYNTAX;

				//send WTX response
				sendBlock(rPCB,cid,rbuffer+RapduLen+rvstat-1,1);
			}
			else//syntax error 			
				 return ERR_RECEIVE_SYNTAX;							
		}
		else if(ERR_RECEIVE_DATOVER==rvstat)
		{
			return 	ERR_RECEIVE_SYNTAX;	
		}
		else if(countNAK<MAX_NAK)//send NAK only when timeout or transmission error
		{
			if(ICCchaining)//If ICCchaining, send ACK
				sendBlock(((mypicc[cid].blocknumber^0x01)|0xA0),cid,tbuffer,0);
			else
			{
				//else, send RNAK 
				if(countNAK<MAX_NAK-1)
					sendBlock(((sPCB|0xF0)&0xBF),cid,tbuffer,0);
				else
					return 	ERR_RECEIVE_SYNTAX;
			}
			countNAK+=1;
		}
		else
			return 	ERR_RECEIVE_SYNTAX;			
	}//end of while(1)	
} //end of ISO14443_sendAPDU

/*******************************************************************************
* Function    : ISO14443_HLTA
* Description : Set PICC to HALT Status Type A
* Arguments   : 			
* Return      :	
*******************************************************************************/
void ISO14443_HLTA()
{  
	THM_SetFWT(fwtpoll);
	THM_SendFrame(c_HLTA,2);    // Send 50 00 to Card 	
}

/*******************************************************************************
* Function    : ISO14443_WUPB
* Description : send WUPB
* Arguments   : ATQB Recieve 			
* Return      :	len   --  Length of ATQB 
				Other --- Error
*******************************************************************************/
 s16 ISO14443_WUPB(u8 nslot, u8 *ATQB)
{
	u16 len;
	u8 rvsta;
	THM_SetFWT(fwtpoll);
	
	//set typeB
	THM_WriteReg(CONTROL_TX,0x62);   //Force 10% module  
	delay_ms(1); 
	THM_ChangeProtBaud(TYPE_B,SND_BAUD_106K,RCV_BAUD_106K);

	//send REQB
	THM_SendFrame(c_WUPB,3); 	
	rvsta=THM_WaitReadFrame(&len,ATQB);
	//return error code
	if(rvsta&FEND){	
		if((ATQB[0] != 0x50)||(len<12)||((ATQB[10]&0x08) == 0x08))	 
			return ERR_REV_WUPB;
		else			
      		return len; 
	}
	else if(rvsta&TMROVER)
		return ERR_TIMEOUT_WUPB;                   
	else
		return ERR_REV_WUPB;
}

/*******************************************************************************
* Function    : ISO14443_Attrib
* Description : send Attrib and recieve response
* Arguments   : ATQB    --- ATQB
				rbuffer --- Attrib			
* Return      :	len     ---  Length of Attrib 
				Other   --- Error
*******************************************************************************/
s16	ISO14443_Attrib(u8 *ATQB,u8 mycid,u8 *rbuffer)
{
	u16 rLEN;
	s16 rvstat;
	u8 maxFS;

	u8 attrib[9];
	attrib[0] = 0x1D;	
	attrib[5] = TR0TR1;								//|EOFSOF;
	attrib[6] = (FSDI|(SendBaud<<2)|(RevBaud<<6));
	attrib[7] = 0x01;									//suppport ISO14443-4
	attrib[8] = mycid;

	memcpy(attrib+1,ATQB+1,4);

	//init PICC
	initPICC(mycid);
	//set PICC structure parameters
	mypicc[mycid].TYPE=TYPE_B;
	mypicc[mycid].CID=mycid;
	mypicc[mycid].DSDR=ATQB[9];

	//set FSCI from ATQB
	maxFS=(ATQB[10]>>4);
	if(maxFS>=8)
		mypicc[mycid].FSCI=8;
	else
		mypicc[mycid].FSCI=maxFS;
	if((ATQB[11]&0xF0) == 0xF0)
		mypicc[mycid].FWI = 0x04;
	else
		mypicc[mycid].FWI=((ATQB[11]&0xF0)>>4);		//set FWI

		THM_SetFWT(1<<mypicc[mycid].FWI);	
	
	//send Attrib command
	THM_SendFrame(attrib,9); 						// Send to Card
	rvstat=THM_WaitReadFrame(&rLEN,rbuffer);
	
	//assign transmission parameters
	if(rvstat&FEND)
	{
		if((rbuffer[0]&0x0F)!=0)//if CID!=0
			return ERR_REV_ATTRIB_RESPONSE;

		//in EMV, CID is not used
		mypicc[mycid].MBLI=(rbuffer[0]>>2); //

		return rLEN;
	}//	if ((rvsta==TMROVER)||(rvsta&FERR)||(rvsta&CERR)||((ATSlen<4)&&(rvsta&PERR))||((ATSlen<4)&&(rvsta&CRCERR))){

	else if ((rvstat&FERR)||(rvstat&CERR)||(rvstat&CRCERR))  
		return ERR_FE_REV_ATTRIB;
	else if (rvstat&TMROVER)
		return ERR_TIMEOUT_REV_ATTRIB;
	else 
		return ERR_REV_ATTRIB_RESPONSE;
}

/*******************************************************************************
* Function    : ISO14443_Anticollision_TypeB
* Description : Type B anticollision
* Arguments   : rbuffer --- Attrib			
* Return      :	len     --- Length of Attrib 
				Other   --- Error
*******************************************************************************/
//type B anticollision
s16	ISO14443_Anticollision_TypeB(u8 cid,u8 *rbuffer)
{
	u8 rATQB[20];
	s16 rvsta;
	u8 i;									    

	rvsta=ISO14443_WUPB(0,rATQB);
	//if timeout, resent WUPB twice
	for(i=0;i<MAX_RETRANS_WUPAB;i++){
		if(rvsta==ERR_TIMEOUT_WUPB)
			rvsta=ISO14443_WUPB(0,rATQB);
	}
	if(rATQB[10]&0x08)
		return ERR_REV_WUPB;
	if(rvsta<0)	return rvsta;
	
	//send attrib
    delay_ms(fdtpcd);
    rvsta=ISO14443_Attrib(rATQB, cid,rbuffer);
	//if timeout, or noise received, resent attrib
	if(rvsta==ERR_TIMEOUT_REV_ATTRIB)
		rvsta=ISO14443_Attrib(rATQB, cid,rbuffer);
	
	return rvsta;
}

/*******************************************************************************
* Function    : ISO14443_Anticollision_TypeB_Analog
* Description : Type B anticollision  for EGT test
* Arguments   : rbuffer --- Attrib			
* Return      :	len     --- Length of Attrib 
				Other   --- Error
*******************************************************************************/
s16	ISO14443_Anticollision_TypeB_Analog(u8 cid,u8 *rbuffer)
{
	u8 rATQB[20];
	s16 rvsta;
									    	
	//send attrib
    delay_ms(fdtpcd);
    rvsta=ISO14443_Attrib(rATQB, cid,rbuffer);
	//if timeout, or noise received, resent 
	if(rvsta==ERR_TIMEOUT_REV_ATTRIB)
	{
		rvsta=ISO14443_Attrib(rATQB, cid,rbuffer);
			if(rvsta==ERR_TIMEOUT_REV_ATTRIB)
			rvsta=ISO14443_Attrib(rATQB, cid,rbuffer);
	}	
	return rvsta;
}


/*******************************************************************************
* Function    : checkBCC
* Description : BCC check , 0 is OK, 1 is failed
* Arguments   : bBCCCheck --- Result of BCC
				pbUID    ---Data to be Check			
* Return      :	0 --- OK
				1 --- Failed
*******************************************************************************/
u8 checkBCC(u8 bBCCCheck, u8 *pbUID)
{
	u8 bBCC;
	bBCC = *(pbUID) ^ *(pbUID+1) ^ *(pbUID+2) ^ *(pbUID+3);
	if(bBCCCheck == bBCC)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*******************************************************************************
* Function    : sendAC
* Description : Send AC Command and recieve CID
* Arguments   : caslevel  ---  Level 
				Iddata    ---  CID Recieved			
* Return      :	4     --- CID Length
				other --- Error
*******************************************************************************/
s16 sendAC(unsigned char caslevel, unsigned char *Iddata)
{
	u8 Tempbuf[7];
	u8 curReceivePostion, lastPostion;
	u8 R_sta;
	u16 iLen;

	Tempbuf[0] = caslevel;
	Tempbuf[1] = 0x20;

	curReceivePostion = 0;
	lastPostion = 0;

	while(1)
	{
		THM_SendFrame(Tempbuf,curReceivePostion + 2);     //send   Anticollision cmd
		//iLen = 5;
		R_sta = THM_WaitReadFrame(&iLen, &Tempbuf[lastPostion + 2]);	// Recieve correct  CID and conflict part
		curReceivePostion = lastPostion + iLen;
		if (iLen != 0)
			lastPostion += iLen - 1;	//Remove last Byte because it maybe conflict Byte or Xor

		if (R_sta & CERR)
		{ 
			if(HandleCollision){
				//conflict and adjust NVB
				Tempbuf[1] = THM_ReadReg (BPOS) + 1;	//bitPos
				Tempbuf[1] += (unsigned char)(iLen+1)<<4;	//bytePOs
				if((Tempbuf[1] & 0x0f)== 0x08)
				{
					Tempbuf[1] = ((Tempbuf[1]&0xf0)+0x10);
					lastPostion = (lastPostion+1);
				}
			}else
				return ERR_RECEIVE_OTHER;
		}
		else if( (R_sta & FEND)||(R_sta & CRCERR))
		{
			if (lastPostion == 4)
			{
				if(checkBCC(Tempbuf[6], &Tempbuf[2]))//BCC error BUG: T302-02
				{
					return ERR_RECEIVE_OTHER;
				}
			    memcpy(Iddata, &Tempbuf[2],5);
				R_sta = 1;
				//break;
				return 4;//return length of UID received includes CT if presents
			}
			else 	
				return ERR_RECEIVE_OTHER; 			
		}
		else if (R_sta & TMROVER)
			return ERR_RECEIVE_TIMEOVER;		
		else if (R_sta & PERR)
			return ERR_RECEIVE_OTHER;				
		else		
			return ERR_RECEIVE_OTHER;
	}//end of while loop
}

/*******************************************************************************
* Function    : initPICC
* Description : Initial PICC
* Arguments   : PICC select with mycid			
* Return      :	
*******************************************************************************/
void initPICC (u8 mycid)
{	
	mypicc[mycid].TYPE= TYPE_A;
	mypicc[mycid].CID = mycid;
	mypicc[mycid].DSDR = 0x00;
	mypicc[mycid].FSCI = 0x02;
	mypicc[mycid].FWI = 0x04;
	mypicc[mycid].SFGI = 0x00;
	mypicc[mycid].WTXM = 0x01;
	mypicc[mycid].withCID = 0x00;
	mypicc[mycid].blocknumber = 0x02;
	mypicc[mycid].MBLI = 0;		
}

/*******************************************************************************
* Function    : sendBlock
* Description : send I block, SBlock, Rblock
* Arguments   : tbuffer --- Send Buffer
				length  --- Send Length
* Return      :	
*******************************************************************************/
static void sendBlock(u8 PCB,u8 cid,u8 *tbuffer,u16 length)  
{
		u8 tempbuffer[261];
		u8 blength;
		//set FWT before send a frame
		THM_SetFWT((1<<mypicc[cid].FWI)*mypicc[cid].WTXM);
	   	
		if(mypicc[cid].withCID)
		{
		 	tempbuffer[0]=(PCB|0x08);
			tempbuffer[1]= mypicc[cid].CID;
			memcpy(tempbuffer+2,tbuffer,length);
			blength=length+2;
		}
		else
		{  		    
		 	tempbuffer[0]=(PCB&0xF7);
			memcpy(tempbuffer+1,tbuffer,length);
			blength=length+1;
		}
		
		THM_SendFrame(tempbuffer,blength);
} 

/*******************************************************************************
* Function    : recvBlock
* Description : RBlock
* Arguments   : rbuffer --- Recieve Buffer
				rLEN    --- Recieve Length
* Return      :	Recieve Length
*******************************************************************************/
static s16  recvBlock(u8 *PCB,u8 *CID,u16 *rLEN,u8 *rbuffer){
 	u8 rvsta; 
	u8 tempbuffer1[300];

	rvsta=THM_WaitReadFrame(rLEN,tempbuffer1);
	if (rvsta & FEND){
		*PCB=tempbuffer1[0];
		//if CID following PCB
		if (((*PCB&0x08) == 0x08)&&(*rLEN>1)){
			*CID=tempbuffer1[1];
			*rLEN -= 2;
			memcpy(rbuffer, (tempbuffer1+2), (*rLEN));
		}else{
			*rLEN-=1;
			memcpy(rbuffer, (tempbuffer1+1), (*rLEN));
		}	
		return *rLEN;	
	}	
	else if(rvsta & DATOVER)
	{
		return ERR_RECEIVE_DATOVER;
	}	
	else
	{
		return ERR_RECEIVE_OTHER;
	}
}  

/*******************************************************************************
* Function    : ISO14443_PPS
* Description : Send pps Command and change Communication Baud Rate
* Arguments   : Type       ---  A or B
				Send_Rate  ---  00:106K  01:212K   02:424K   03:848K
				Rec_Rate   ---  00:106K  01:212K   02:424K   03:848K			
* Return      :	1  --- PPS  OK
				0  --- PPS Failed
*******************************************************************************/
s16 ISO14443_PPS(u8  Type,u8 Send_Rate,u8 Rec_Rate)
{
	u16 len;
	unsigned char PPS[] = {0xd0,0x11,0x00};
	PPS[2] = (PPS[2]|(Rec_Rate<<2)|Send_Rate);

	SendBaud =	(Send_Rate<<2);
	RevBaud = Rec_Rate;

	THM_SendFrame(PPS,3);
	THM_WaitReadFrame(&len,PPS_TEMP);
	if(PPS_TEMP[0]==PPS[0])
	{
		if(Type == 0x0A)
			THM_ChangeProtBaud(TYPE_A,SendBaud,RevBaud);
		else if	(Type == 0x0B)
			THM_ChangeProtBaud(TYPE_B,SendBaud,RevBaud);
		return 1;
	} 
	else
		return 0;		
}

/*******************************************************************************
* Function    : delay_ms
* Description : delay about 0.6ms
* Arguments   : ms  --- number of 0.6ms			
* Return      :	
*******************************************************************************/
void delay_ms(unsigned int ms)
{
	unsigned int i,j,k;
	k = 7975;
	for(i=0;i<ms;i++)
	{
		for(j=0;j<k;j++)
		{}
	}
}

