#include <msp430.h>

/******************** 測試版 HW ***************************
#define    CLE  BIT2    //設置CLE為輸出管腳 P3.0
#define    CE   BIT3    //設置CE~為輸出管腳 P3.2
#define    ALE  BIT1    //設置ALE為輸出管腳 P3.1
#define    RE   BIT4    //設置RE~為輸出管腳 P3.3
#define    WE   BIT0    //設置WE~為輸出管腳 P3.6
#define    WP   BIT5    //設置WP為輸出管腳 P3.7
#define    RB   BIT7 //設置R/B為輸入管腳 P2.7
*/
//******************** 08 HW ***************************
#define    CLE  BIT0    //設置CLE為輸出管腳 P3.0
#define    CE   BIT2    //設置CE~為輸出管腳 P3.2
#define    ALE  BIT1    //設置ALE為輸出管腳 P3.1
#define    RE   BIT3    //設置RE~為輸出管腳 P3.3
#define    WE   BIT6    //設置WE~為輸出管腳 P3.6
#define    WP   BIT7    //設置WP為輸出管腳 P3.7
#define    RB   BIT7    //設置R/B為輸入管腳 P2.7
//******************** 08 HW ***************************/
#define    FLASHControl   P3OUT
#define    FLASHBus       P3DIR
#define    FLASHDATAOUT   P1OUT
#define    FLASHDATAIN    P1IN
#define    FLASHDIR       P1DIR
#define    RBPort         P2DIR
#define    RBIO           P2IN

#define CLE_Enable    {FLASHControl |= CLE;}
#define CLE_Disable   {FLASHControl &= ~CLE;}
#define CE_Enable     {FLASHControl &= ~CE;}
#define CE_Disable    {FLASHControl |= CE;}
#define ALE_Enable    {FLASHControl |= ALE;}
#define ALE_Disable   {FLASHControl &= ~ALE;}
#define RE_Enable     {FLASHControl &= ~RE;}
#define RE_Disable    {FLASHControl |= RE;}
#define WE_Enable     {FLASHControl &= ~WE;}
#define WE_Disable    {FLASHControl |= WE;}

#define PAGE_SIZE             	528            /* Page size in byte */
#define PAGE_DATA_SIZE        	512            /* Page data size in byte */
#define PAGE_SPARE_SIZE       	16             /* Spare page size in byte */
#define BACKUP_BUF_SIZE       	64             

extern void Init_Flash(void);
extern void Reset_Flash(void);
extern void SendFlashCMD(unsigned char CMD);
extern void AccessAddress(unsigned char nCol,unsigned long nRow);
extern void PageRead(unsigned int nCol,unsigned long nRow,unsigned char *pBuf,unsigned int Len);
extern int PageWrite(unsigned int nCol,unsigned long nRow,unsigned char *pBuf,unsigned int Len);
extern int WriteByte(unsigned int nCol,unsigned long nRow,unsigned char nValue);
extern unsigned char ReadByte(unsigned int nCol,unsigned long nRow);
extern void S_ECC_gen(unsigned char *Buffer, unsigned char *ECC_code);
extern void ECC_gen(unsigned char *Buffer,unsigned char *ECC_code);
extern void FRead_Data(unsigned long Page_index,unsigned char *Databuff);
extern void FSave_Data(unsigned long Page_index,unsigned char *Databuff);

