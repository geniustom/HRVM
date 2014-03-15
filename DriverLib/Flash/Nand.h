#include <msp430.h>

/******************** ���ժ� HW ***************************
#define    CLE  BIT2    //�]�mCLE����X�޸} P3.0
#define    CE   BIT3    //�]�mCE~����X�޸} P3.2
#define    ALE  BIT1    //�]�mALE����X�޸} P3.1
#define    RE   BIT4    //�]�mRE~����X�޸} P3.3
#define    WE   BIT0    //�]�mWE~����X�޸} P3.6
#define    WP   BIT5    //�]�mWP����X�޸} P3.7
#define    RB   BIT7 //�]�mR/B����J�޸} P2.7
*/
//******************** 08 HW ***************************
#define    CLE  BIT0    //�]�mCLE����X�޸} P3.0
#define    CE   BIT2    //�]�mCE~����X�޸} P3.2
#define    ALE  BIT1    //�]�mALE����X�޸} P3.1
#define    RE   BIT3    //�]�mRE~����X�޸} P3.3
#define    WE   BIT6    //�]�mWE~����X�޸} P3.6
#define    WP   BIT7    //�]�mWP����X�޸} P3.7
#define    RB   BIT7    //�]�mR/B����J�޸} P2.7
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

