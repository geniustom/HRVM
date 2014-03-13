/* Basic Data-type*/
typedef unsigned char        	ubyte;		/*位元*/
typedef signed char          	byte;
typedef unsigned short       	uword;		/*字元*/
typedef signed short         	word;
typedef unsigned long int      	udword;		/*雙字元*/
typedef signed long int        	dword;	

/* SAMSUNG NAND FLASH K9F1208U0B */
#define FLASH_WIDTH           	8              /* Flash data width */
#define FLASH_SIZE            	0x4000000      /* Flash size in byte */
#define PAGE_SIZE             	528            /* Page size in byte */
#define PAGE_DATA_SIZE        	512            /* Page data size in byte */
#define PAGE_SPARE_SIZE       	16             /* Spare page size in byte */
#define NUM_BLOCKS				4096           /* Number of blocks in device*/
#define HALF_PAGE_POINTER     	(ubyte) 0x01   /* Half page Pointer */
#define SHIFT_A8		1
typedef ubyte dataWidth;                     	/* Flash data type */



//------------IN ECC.C
void ECC_gen(dataWidth *Buffer, dataWidth *ECC_code);
void S_ECC_gen(dataWidth *Buffer, dataWidth *ECC_code);






/*******************************************************************************
                                Basic functions
*******************************************************************************/
void NAND_ReSet();
void NAND_Open(void);
void NAND_CommandInput(ubyte ubCommand);    	/* Put a command on bus */
void NAND_AddressInput(ubyte ubAddress);    	/* Put an address on bus */
void NAND_DataInput(dataWidth ubData);      	/* Write a data to flash */
dataWidth NAND_DataOutput(void);           		/* Read a data from the flash */
void NAND_Close(void);                      	/* Close the command after an */
                                            	/* operation is completed. */

/*******************************************************************************
			          Return Codes
*******************************************************************************/
typedef ubyte NAND_Ret;

#define NAND_PASS                  0x00 	/* the operation on the NAND was
                                            	   successfully completed*/
#define NAND_FAIL                  0x01 	/* the operation on the nand failed */
#define NAND_FLASH_SIZE_OVERFLOW   0x02 	/* the address is not within the device*/
#define NAND_PAGE_OVERFLOW         0x04 	/* attempt to access more than one page*/
#define NAND_WRONG_ADDRESS         0x08 	/* the address is not */

/******************************************************************************
                            Nand operation functions
******************************************************************************/

NAND_Ret NAND_BlockErase(udword udAddress);

NAND_Ret NAND_CopyBack(udword udSourceAddr,udword udDestinationAddr);

NAND_Ret NAND_PageRead(udword udAddress, dataWidth *Buffer, udword udlength);

NAND_Ret NAND_PageProgram(udword udAddress, dataWidth *Buffer, udword udlength);

NAND_Ret NAND_SpareProgram(udword udAddress, dataWidth *Buffer, udword udlength);

void NAND_ReadElectronicSignature(dataWidth *Buffer);

ubyte NAND_ReadStatusRegister(void);

void NAND_Reset(void);

NAND_Ret NAND_SpareRead(udword udAddress, dataWidth *Buffer, udword udlength);








