#include "PortManager.h"
#include "AccessFlash.h"


u32 Page=32;
u8 Sparebuff[16]={0};
u8 Sparebuff1[16]={0};

///////////////////////////////////////////////////////////////////
// save_data(u32 Page_index,u8 *Databuff)                  //
// input: Page_index: Page_number that want to save(0~131072)    //
//        Databuff: Data buffer that want to save		 //
///////////////////////////////////////////////////////////////////
void save_data(u32 Page_index,u8 *Databuff)
{
  u32 data_address,Spare_address;

  if (Page_index==0)
  {
    data_address = Page_index*(PAGE_SIZE);		//transform the Page_index to Address
    Spare_address = data_address + PAGE_DATA_SIZE;	//transform the Page_index to Spare_address
  }
  else
  {
    data_address = Page_index*(PAGE_SIZE);		//transform the Page_index to Address
    Spare_address = data_address + PAGE_DATA_SIZE-1;	//transform the Page_index to Spare_address
  }

  ECC_gen(Databuff,Sparebuff);		   		//generator ECC code for main area data	
  S_ECC_gen(Sparebuff,Sparebuff);		   	//generator ECC code for spare area data
////////////////////////////////////////////////////////////////
  	
  //NAND_ReadElectronicSignature(DC); //讀取ID

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//	Name: NAND_BlockErase()				      //
//	Function: Erase one Block			      //
//	Describe: One block include 32 Page		      //
////////////////////////////////////////////////////////////////
/*
  if(NAND_BlockErase(data_address)!=0) 	   			    //BLOCK ERASE(重新寫入前需先抹除)
  {
  	
  	_NOP();//fail //to do
  }
  else
  {
  	_NOP();//pass //to do
  }	
*/
///////////////////////////////////////////////////////////////
  if(NAND_SpareProgram(Spare_address,Sparebuff,PAGE_SPARE_SIZE)!=0) //寫入ECC Code
  {
  	_NOP();//fail //to do
  }
  else
  {
  	_NOP();//pass //to do
  }
///////////////////////////////////////////////////////////////

  if(NAND_PageProgram(data_address,Databuff,PAGE_DATA_SIZE)!=0) //將DC陣列內之資料寫入，並確認是否有Error
  {
  	_NOP();//fail //to do
  }
  else
  {
  	_NOP();//pass //to do
  }
  
}

///////////////////////////////////////////////////////////////////
// read_data(u32 Page_index,u8 *Databuff)                  //
// input: Page_index --> Page_number that want to read(0~131072) //
// output: Databuff --> Data buffer from the NANDFLASH		 //
///////////////////////////////////////////////////////////////////
void read_data(u32 Page_index,u8 *Databuff)
{
  u32 data_address,Spare_address;



  if (Page_index==0)
  {
  	data_address = Page_index*(PAGE_SIZE);			//transform the Page_index to Address
    	Spare_address = data_address + PAGE_DATA_SIZE;	        //transform the Page_index to Spare_address
  }
  else
  {
    	data_address = Page_index*(PAGE_SIZE);			//transform the Page_index to Address
    	Spare_address = data_address + PAGE_DATA_SIZE-1;	//transform the Page_index to Spare_address
  }
///////////////////////////////////////////////////////////////////////////////
//			spare area ECC code check
///////////////////////////////////////////////////////////////////////////////
 _NOP();
 if(NAND_SpareRead(Spare_address,Sparebuff,PAGE_SPARE_SIZE)!=0) //讀取舊Spare area 內之data為ECC
 {
 	_NOP();//fail //to do
 }
 else
 {
  	_NOP();//pass
 }
/////////////////////////////////////////////////
  u8 data_buff1,data_buff2;
  data_buff1=Sparebuff[9];
  data_buff2=Sparebuff[10];			   		//set S_ECC to 0
  Sparebuff[9]=0X00;
  Sparebuff[10]=0X00;	
  S_ECC_gen(Sparebuff,Sparebuff1); 		   		//產生S_ECC code為ECC1
  _NOP();
  Sparebuff[9]=data_buff1;
  Sparebuff[10]=data_buff2;
/////////////////////////////////////////////////
 int j=0,i1=0,i2=0;
	i1=Sparebuff[9]^Sparebuff1[9]; 	   			//XOR the ECC code
	i2=Sparebuff[10]^Sparebuff1[10];
	j=i1|i2;
 if (j==0)
 {
	_NOP();//pass 	//if result is zero means Pass no error occur
 }
 else
 {
	_NOP();//fail error occur //to do
 }
//////////////////////////////////////////////////////////////////////////////
//	main data area ECC code check
//////////////////////////////////////////////////////////////////////////////
 _NOP();
 NAND_PageRead(data_address,Databuff,PAGE_DATA_SIZE); 	        //讀取資料
 _NOP();
/////////////////////////////////////////////////

  ECC_gen(Databuff,Sparebuff1); 		   		//產生新碼ECC code為ECC1
 _NOP();
/////////////////////////////////////////////////
NAND_SpareRead(Spare_address,Sparebuff,PAGE_SPARE_SIZE);	//讀取舊ECC code為ECC
/////////////////////////////////////////////////
 int L=0,l1=0,l2=0,l3=0;
	l1=Sparebuff[6]^Sparebuff1[6]; 	   			//XOR the ECC code
	l2=Sparebuff[7]^Sparebuff1[7];
	l3=Sparebuff[8]^Sparebuff1[8];
	L=l1|l2|l3;
 if (L==0)
 {
   _NOP();//pass //if result is zero means Pass no error occur
 }
 else
 {
    _NOP();//fail error occur //to do
 }
///////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////
//	Name: NAND_BlockErase()	  			      //
//	Function: Erase one Block		  	      //
//	Describe: One block include 32 Page	              //
////////////////////////////////////////////////////////////////
void Block_Erase(u32 Page_index)
{
  u32 data_address;
  if (Page_index==0)
  {
    data_address = Page_index*(PAGE_SIZE);		//transform the Page_index to Address
  }
  else
  {
    data_address = Page_index*(PAGE_SIZE);		//transform the Page_index to Address
  }
  if(NAND_BlockErase(data_address)!=0) 	   		//BLOCK ERASE(重新寫入前需先抹除)
  {  	
  	_NOP();//fail //to do
  }
  else
  {
  	_NOP();//pass //to do
  }	
}




////// CLOCK_SET 副程式 //////
//////////////////////////////////////////////////////////////////////////////
//	function name:CLOCK_SET()		 			    //	
//	function:set system clock source as XT2, SMCLK is (MCLK/4)=1MHZ     //
//	return:NaN		 					    //
//////////////////////////////////////////////////////////////////////////////
void CLOCK_SET (void)
{
  volatile unsigned int i;
  WDTCTL = WDTPW + WDTHOLD;                 			// Stop WDT
  P5DIR |= 0x10;                            			// P5.4= output direction
  P5SEL |= 0x10;                            			// P5.4= MCLK option select
  BCSCTL1 &= ~XT2OFF;                       			// XT2= HF XTAL

  do
  {
    IFG1 &= ~OFIFG;                        	 		// Clear OSCFault flag
    for (i = 0xFF; i > 0; i--);             			// Time for flag to set
  }
  while ((IFG1 & OFIFG));                   			// OSCFault flag still set?

  BCSCTL2 |= SELM_2+SELS;                   			// MCLK = SMCLK = XT2 (safe)
}
