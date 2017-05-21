#ifndef __CRYPTO1_H
    #define __CRYPTO1_H

#define MBLOCK_SIZE   0x10
#define MWALLET_SIZE  0x04

unsigned char THM_MFindCard(unsigned char * b_uid);
unsigned char THM_MFindCard_A(unsigned char * b_uid);
void THM_MInitialKEY(unsigned char type, unsigned char * key);
unsigned char THM_MAuthentication(unsigned char bKeyNo,unsigned char bBlock);
unsigned char THM_MAuthenticationT(unsigned char bKeyNo,unsigned char bBlock);
unsigned char THM_MRead (unsigned char bBlock,unsigned char* pbLen,unsigned char *pdat );
unsigned char THM_MWrite(unsigned char bBlock,unsigned char *dat);

unsigned char THM_MTransfer(unsigned char bBlock);
unsigned char THM_MRestore(unsigned char bBlock);

unsigned char THM_MIncrement(unsigned char bBlock,unsigned char * pbValue );
unsigned char THM_MDecrement(unsigned char bBlock,unsigned char *pbValue );

unsigned char THM_MReadValue(unsigned char bBlock,unsigned char * pbValue,unsigned char * pbAddr);
unsigned char THM_MWriteValue(unsigned char bBlock,unsigned char * pbValue,unsigned char bAddr);
extern void THM_MSendFrame(unsigned char *buffer,unsigned char num);
extern unsigned char THM_MWaitReadFrame(unsigned char *len, unsigned char *buffer);

unsigned char THM_MWriteSectorTrailer(unsigned char bBlock,unsigned char* pbKEYA, unsigned char * pbKEYB, unsigned char * bAccessBits); 

void THM_MHLTA(void);

#endif

