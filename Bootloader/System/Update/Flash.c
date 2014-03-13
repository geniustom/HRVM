#include "Flash.h"
#include "PortManager.h"






/******************************************************************************
P3.0 ---> CLE	P3.1 ---> ALE	P3.2 ---> CE	P3.3 ---> RE	P3.6 ---> WE	
P3.7 ---> WP	P2.7 ---> R/B	
P1.0 ~ P1.7 ---> I/O.0 ~ I/O.7
******************************************************************************/
void NAND_ReSet(){
  NAND_Close();
  NAND_Open();
}


/******************************************************************************
			   ENABLE THE CHIP
******************************************************************************/
void NAND_Open()
{
/****** PORT Initial ******/	
 P1DIR = 0XFF;		
 P1IE = 0X00;
 P1SEL = 0X00;
 P1OUT = 0X00;	
	
 P3DIR = 0XCF;		
 P3SEL = 0X30;
 P3OUT = 0X00;
}
/*******************************************************************************
                           NAND_CommandInput
*******************************************************************************/
void NAND_CommandInput(u8 ubCommand)
{
 P1DIR = 0XFF;
 P3DIR = 0XCF;
 CONTROL = COMMAND_LATCH;
 DATA_OUT = ubCommand;
 CONTROL &= CE;
 CONTROL &= ALE;
 CONTROL |= (~CLE);
 CONTROL &= WE;
 CONTROL |= (~WE);
 CONTROL &= CLE;
 CONTROL |= (~ALE);
}
/******************************************************************************
                  	   NAND_AddressInput
*******************************************************************************/
void NAND_AddressInput(u8 ubAddress)
{
 P1DIR = 0XFF;
 P3DIR = 0XCF; 	
 CONTROL = ADDRESS_LATCH;
 DATA_OUT = ubAddress; /*PUT ADDRESS INTO DATA BUS*/
 CONTROL &= CE;
 CONTROL &= CLE;
 CONTROL |= (~ALE);
 CONTROL &= WE;
 CONTROL |= (~WE);
 CONTROL &= ALE;
}
/******************************************************************************
                           NAND_DataInput
******************************************************************************/
void NAND_DataInput(dataWidth ubData)
{
 P1DIR = 0XFF;
 P3DIR = 0XCF;
 CONTROL = INPUT_DATA_LATCH;
 DATA_OUT = ubData;
 CONTROL &= WE;
 CONTROL |= (~WE);
}
/******************************************************************************
                  	   NAND_DataOutput
*******************************************************************************/
dataWidth NAND_DataOutput()
{
 u8 BUFFER;
 P1DIR = 0X00;
 P3DIR = 0XCF; 	
 CONTROL = OUTPUT_DATA_LATCH;
 CONTROL &= RE;
 BUFFER = DATA_IN;
 CONTROL |= (~RE);
 return (BUFFER);
}
/******************************************************************************
                           InsertAddress
******************************************************************************/
void InsertAddress(u32 udAddress)
{
 u32 udAddress1=0,page_address=0,block_address=0;	
 page_address = (udAddress / PAGE_SIZE)%32;	
 block_address = (udAddress /PAGE_SIZE)/32; 	
 udAddress1 = udAddress1 | (page_address<<8) |(block_address<<13);	
 P1DIR = 0XFF;
 P3DIR = 0XCF;
 CONTROL = INSERT_ADDRESS_LATCH;
 DATA_OUT = udAddress1;
 CONTROL &= CE;
 CONTROL &= CLE;
 CONTROL |= (~ALE);
 CONTROL &= WE;
 CONTROL |= (~WE);
 DATA_OUT = udAddress1>>8;
 CONTROL &= WE;
 CONTROL |= (~WE);
 DATA_OUT = udAddress1>>16;
 CONTROL &= WE;
 CONTROL |= (~WE);
 DATA_OUT = udAddress1>>24;
 CONTROL &= WE;
 CONTROL |= (~WE);
 CONTROL &= ALE;
}
/******************************************************************************
                 InsertColumnAddress
******************************************************************************/
void InsertColumnAddress(u32 udAddress)
{
 u32 udAddress1=0,page_address=0,block_address=0;	
 page_address = (udAddress / PAGE_SIZE)%32;	
 block_address = (udAddress /PAGE_SIZE)/32; 	
 udAddress1 = udAddress1 | (page_address<<8) |(block_address<<13);
 P1DIR = 0XFF;
 P3DIR = 0XCF;
 CONTROL = INSERT_ADDRESS_LATCH;
 DATA_OUT = udAddress1>>8;
 CONTROL &= CE;
 CONTROL &= CLE;
 CONTROL |= (~ALE);
 CONTROL &= WE;
 CONTROL |= (~WE);
 DATA_OUT = udAddress1>>16;
 CONTROL &= WE;
 CONTROL |= (~WE);
 DATA_OUT = udAddress1>>24;
 CONTROL &= WE;
 CONTROL |= (~WE);
 CONTROL &= ALE;
}
/******************************************************************************
                           NAND_Close
******************************************************************************/
void NAND_Close()
{
  CONTROL |= (~CE);
}
/*******************************************************************************
                       waitForReady
*******************************************************************************/
u8 waitForReady()
{
 u8 BUFFER;	

 P1DIR = 0XFF;
 P3DIR = 0XCF;
 CONTROL = STATUS_READ_LATCH;
 DATA_OUT = 0X70;
 CONTROL |=(~CLE);
 CONTROL &= CE;
 CONTROL &= WE;
 CONTROL |= (~WE);
 CONTROL &=CLE;
 P1DIR = 0X00;
 CONTROL &= RE;
 BUFFER = DATA_IN;
 CONTROL |= (~RE);
  return (BUFFER);
}
/******************************************************************************
                  NAND_BlockErase                                                           `
******************************************************************************/
NAND_Ret NAND_BlockErase(u32 udAddress)
{
    u8 ubStatus;

  if ( udAddress >= FLASH_SIZE )			
      return NAND_FLASH_SIZE_OVERFLOW;			/* NAND_Ret=0x02 */

  NAND_Open();

/* Issue Sequential data input command */
  NAND_CommandInput((u8)0x60);

  InsertColumnAddress(udAddress);			

/* Issue confirm code command */
  NAND_CommandInput((u8)0xD0);

Wait:
   /* Wait for ready */
   ubStatus=waitForReady();				/* wait fo not busy */
   if(ubStatus&(0x40)==0x40)
   {
    goto Wait;//busy	
   }
  NAND_Close();//fail or ok
  return ubStatus&(0x01);				/* RETURN DEVICE STATUS */						
}

/******************************************************************************
                  NAND_PageRead
*******************************************************************************/
NAND_Ret NAND_PageRead(u32 udAddress, dataWidth *Buffer, u32 udlength)
{

   volatile u32 udIndex;							
   u8 ubStatus;
   u16 uwFarToPage;

   /* Control if the address is within the flash*/
   if ( udAddress >= FLASH_SIZE )				
    return NAND_FLASH_SIZE_OVERFLOW;

   NAND_Open();

   /* Set the address pointer*/
   if ( ((udAddress%PAGE_SIZE) % PAGE_DATA_SIZE) >= PAGE_DATA_SIZE>>1 )	/* A8 =1 */
    NAND_CommandInput(HALF_PAGE_POINTER);   		/* 2nd half page*/
   else
    NAND_CommandInput((u8)0x00);         		/* 1st half page*/

   /*send the address*/
  InsertAddress(udAddress);					

Wait:
   /* Wait for ready */
   ubStatus=waitForReady();				/* wait fo not busy */
   if(ubStatus&(0x40)==0x40)
     {
      goto Wait;//busy
     }
   if(ubStatus&(0x01)==0x01)
     {
      goto Close;//fail
     }
   /* Leave Read Status Register mode */
   NAND_CommandInput((u8)0x00);


   /* Calculate the distance for the page edge*/
   uwFarToPage = (PAGE_SIZE  - (udAddress % PAGE_DATA_SIZE));

   /* Read data to the internal buffer */
   udIndex=0;
   while(udIndex<udlength)				
     if ( uwFarToPage == 0 )
     {
	/*Wait fo Ready*/
        ubStatus=waitForReady();
	
	/* Leave Read Status Register mode */
	NAND_CommandInput((u8)0x00);
	uwFarToPage=PAGE_SIZE;		
      }
      else
      {
	Buffer[udIndex++] = NAND_DataOutput();
	uwFarToPage--;
      }
Close:	
   NAND_Close();
   /* Return Pass or Fail */
   return (ubStatus&(0x01));
}

/******************************************************************************
                NAND_PageProgram function
******************************************************************************/
NAND_Ret NAND_PageProgram(u32 udAddress, dataWidth *Buffer, u32 udlength)
{
  register u32 udIndex;
  u8 ubStatus;

  /* Control if the address is within the flash*/
  if ( udAddress >= FLASH_SIZE )
    return NAND_FLASH_SIZE_OVERFLOW;

  /* Control if the address is the buffer size is within the page */
  if ( ( (udAddress % PAGE_SIZE) +  udlength ) > PAGE_SIZE )
    return NAND_PAGE_OVERFLOW;

  NAND_Open();
	
  /* Check for A8 - Set the address pointer */
  /* A8 =1*/
  if ( ((udAddress%PAGE_SIZE) % PAGE_DATA_SIZE) >= PAGE_DATA_SIZE>>1 )
    /* 2nd half page*/
    NAND_CommandInput(HALF_PAGE_POINTER);
  else
    /* 1st half page*/
    NAND_CommandInput((u8)0x00);
	
    /* Issue Sequential data input command */
    NAND_CommandInput((u8)0x80);
	
    /*send the address*/
    InsertAddress(udAddress);
	
    /* Write the data to the internal buffer */
    for (udIndex=0;udIndex<udlength;udIndex++)
      NAND_DataInput((Buffer[udIndex]));

    /* Issue the Program Command*/
    NAND_CommandInput((u8)0x10);
Wait:
   /* Wait for ready */
   ubStatus=waitForReady();			/* wait fo not busy */
   if(ubStatus&(0x40)==0x40)
   {
     goto Wait;//busy	
   }
   //NAND_CommandInput((u8)0x00);//leave status register for test
   NAND_Close();
   /* Return Pass or Fail */
   return (ubStatus&(0x01));
}

/******************************************************************************
                  NAND_Read ID
******************************************************************************/
void NAND_ReadElectronicSignature(dataWidth *Buffer)
{ 	
   u8 ubIndex;
   /* int i;*/
   NAND_Open();

   /* Set the address pointer */
   NAND_CommandInput((u8)0x90);  /* Spare Area*/

   /* Issue the address bytes*/
   NAND_AddressInput((u8)0x00); /* first address byte */

   for (ubIndex=0;ubIndex<2;ubIndex++)
     Buffer[ubIndex]= NAND_DataOutput();

   NAND_Close();
}

/******************************************************************************
                    NAND_ReadStatusRegister
*******************************************************************************/
u8 NAND_ReadStatusRegister()
{
   u8 ret;
   NAND_Open();
   /* Issue Read Status Register command */
   NAND_CommandInput((u8)0x70);
   ret = (u8)NAND_DataOutput();

   NAND_Close();
   /* Return the Status Register */
   return ret;
}

/************************  NAND_ReadStatusRegister ****************************/

/******************************************************************************
                      NAND_Reset
******************************************************************************/
void NAND_Reset(void) {

   NAND_Open();
   /* Issue the Reset Command*/
   NAND_CommandInput((u8)0xFF);

   NAND_Close();
}
/******************************  NAND_Reset ***********************************/

/*******************************************************************************
                  NAND_SpareProgram
*******************************************************************************/
NAND_Ret NAND_SpareProgram(u32 udAddress, dataWidth *Buffer, u32 udlength)
{
   u8 ubStatus;
   u32 ubIndex;

   /* Control if the address is within the flash*/
   if ( udAddress >= FLASH_SIZE )
     return NAND_FLASH_SIZE_OVERFLOW;

   /* Control if the address is the buffer size is within the page*/
    if ( ( (udAddress % PAGE_SIZE) +  udlength ) > PAGE_SIZE )
      return NAND_PAGE_OVERFLOW;

   /* Set the address pointer to the Spare Area*/
   NAND_CommandInput((u8)0x50);     /* Spare Area */

   /* Issue Sequential data input command */
   NAND_CommandInput((u8)0x80);

   InsertAddress(udAddress);

   /* Write the data to the internal buffer */
   for (ubIndex=0;ubIndex<udlength;ubIndex++)
     NAND_DataInput(Buffer[ubIndex]);

   /* Issue the Program Command*/
   NAND_CommandInput((u8)0x10);

Wait:
   /* Wait for ready */
   ubStatus=waitForReady();		/* wait fo not busy */
   if(ubStatus&(0x40)==0x40)
   {
     goto Wait;//busy	
   }
   NAND_Close();

   /* Return Pass or Fail */
   return (ubStatus&(0x01));
}
/***************************  NAND_SpareProgram *******************************/

/******************************************************************************
                  NAND_SpareRead
******************************************************************************/
NAND_Ret NAND_SpareRead(u32 udAddress, dataWidth *Buffer, u32 udlength)
{
   u32 udIndex;
   u8 ubStatus;
   u16 uwFarToPage;

   /* Control if the address is within the flash*/
   if ( udAddress >= FLASH_SIZE )
     return NAND_FLASH_SIZE_OVERFLOW;

   /* Set the address pointer */
   NAND_CommandInput((u8)0x50);      /* Spare Area*/

   InsertAddress(udAddress);
Wait:
   /* Wait for ready */
   ubStatus=waitForReady();		/* wait fo not busy */
   if(ubStatus&(0x40)==0x40)
   {
     goto Wait;//busy	
   }

   /* Leave Read Status Register mode */
   NAND_CommandInput((u8)0x50);

   /* Calculate the distance for the page edge*/
   uwFarToPage = (PAGE_SIZE  - (udAddress % PAGE_SIZE));

   /* Read data to the internal buffer */
   udIndex=0;
   while(udIndex<udlength)

   if ( uwFarToPage == 0 )
   {
     ubStatus=waitForReady(); /* Wait for ready*/
     uwFarToPage = PAGE_SPARE_SIZE;

     /* Leave Read Status Register mode */
     NAND_CommandInput((u8)0x50);
   }
   else
   {
     Buffer[udIndex++] = NAND_DataOutput();
     uwFarToPage--;
   }
   NAND_Close();

   /* Return Pass or Fail */
   return (ubStatus&(0x01));
}
