// vim: set ts=4 sw=4 fdm=marker:

#include "usb_lib.h"
#include "usb_bot.h"
#include "usb_istr.h" 

#ifndef __THM3060_H
#define __THM3060_H
    
#define DATA        0x00
#define PSEL        0x01
#define FCONB       0x02
#define EGT         0x03
#define CRCSEL      0x04
#define RSTAT       0x05
#define SCNTL       0x06
#define INTCON      0x07
#define RSCH        0x08
#define RSCL        0x09    
#define CRCH        0x0a
#define CRCL        0x0b
#define BPOS        0x0e
#define SMOD        0x10
#define PWTH        0x11
#define STAT_CTRL   0x12
#define FM_CTRL     0x13
#define UART_STAT   0x14
#define CRYPTO1_CTRL 0x15
#define DATA0       0x16
#define DATA1       0x17
#define DATA2       0x18
#define DATA3       0x19
#define DATA4       0x1A
#define COL_STAT    0x1B
#define SND_CTRL    0x1C
#define EMVEN		0x20
#define FWTH		0x21
#define FWTM		0x22
#define FWTL		0x23
#define AFDTBOUND	0x24
#define EMVERR		0x25
#define TXFIN		0x26
#define RERXEN		0x27
#define TMRH		0x28
#define TMRM		0x29
#define TMRL		0x2a
#define TR0H		0x2e
#define TR0L		0x2f

#define RNG_CON		0x30
#define RNG_STS		0x31
#define RNG_DATA	0x32

//EMVEN bits
#define FDTPICCINVALIDERR	0x01
#define FDTPICCNOISEERR 	0x02
#define FDTTMOUTERR			0x04
#define TR0ERR				0x08
#define TR1ERR				0x10 //typeB noise
#define ALL_NOISEERR		0x20
#define MSK_NOISEERR		0xED

#define CONTROL_TX			0x40
#define DP1					0x41
#define DP0					0x42
#define DN1					0x43
#define DN0					0x44
#endif
#define CTR_RX				0x45
#define	INTER_TRIM1		    0x46
#define	INTER_TRIM2		    0x47
#define	INTER_TRIM3		    0x48

//#define RSTAT bits
#define FEND        0x01
#define CRCERR      0x02
#define TMROVER     0x04
#define DATOVER     0x08
#define FERR        0x10
#define PERR        0x20
#define CERR        0x40

#define TYPE_A      0x10
#define TYPE_B      0x00
#define ISO15693    0x20
#define ETK         0x30
#define MIFARE      0x50
#define SND_BAUD_106K   0x00
#define SND_BAUD_212K   0x04
#define SND_BAUD_424K   0x08
#define SND_BAUD_848K   0x0c

#define RCV_BAUD_106K   0x00
#define RCV_BAUD_212K   0x01
#define RCV_BAUD_424K   0x02
#define RCV_BAUD_848K   0x03

extern void THM_WriteReg(unsigned char address,unsigned char content);
extern unsigned char THM_ReadReg(unsigned char address);
extern unsigned char THM_WaitReadFrame(unsigned short *len, unsigned char *buffer);
extern unsigned char THM_ReadStat( unsigned short *len);
extern void THM_ReadBuff( unsigned short *len,unsigned char * buff);
extern void THM_SendFrame(unsigned char *buffer,unsigned short num);
extern void THM_Reset(void);
extern void THM_ChangeProtBaud(unsigned char prot, unsigned char sndbaud, unsigned char rcvbaud);
extern void SleepMode(u8 Modetype);
void THM_SetFWT(u32 ulFWT);
void THM_EnableEMV(void);
 

