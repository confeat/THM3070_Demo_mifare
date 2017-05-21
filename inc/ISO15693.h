

extern u8 VCDbuf[256];
extern u8  gUID[8];

extern u16 ISO15693_ReadCARD(u8 * buf);
extern u16 ISO15693_Stayquiet(void);
extern u16 ISO15693_WriteBlock(u8 Blocknum ,u8 datalen,u8 * buf);
extern u16 ISO15693_LockBlock(u8 Blocknum);
extern u16 ISO15693_ReadMBlock(u8 Blockstart ,u8 Blocklen,u8 * buf);
extern u16 ISO15693_WriteMBlock(u8 Blockstart ,u8 Blocknum,u8 datalen,u8 * buf);
extern u16 ISO15693_Select(void);
extern u16 ISO15693_Reset2ready(void);
extern u16 ISO15693_WriteAFI(u8 AFI);
extern u16 ISO15693_LockAFI(void);
extern u16 ISO15693_WriteDSFID(u8 DSFID);
extern u16 ISO15693_LockDSFID(void);
extern u16 ISO15693_GetSysInf(u8 * buf);
extern u16 ISO15693_ReadMBlockStatus(u8 Blockstart ,u8 Blocklen,u8 * buf);

extern u16 ISO15693_ReadCARD_WithAFI(u8 * buf,u8 AFI);
