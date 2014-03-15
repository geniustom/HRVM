#include "../../DriverExport.h"


void  flash_write_word (unsigned int value,unsigned int address){
  unsigned int *Flash_ptr= (unsigned int *) address;    // Initialize Flash pointer
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  *Flash_ptr = value;                       // Write value to flash

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void  flash_write_Block (unsigned int address,unsigned char const *FBuf,unsigned int Length){
  unsigned int *Flash_ptr= (unsigned int *) address;    // Initialize Flash pointer
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  for (int i=0;i<Length;i+=2){
    if((FBuf[i]!=0xff)||(FBuf[i+1]!=0xff)){  //如果是0xFF就不更新
      //*Flash_ptr =((unsigned int)FBuf[i]<<8)|(FBuf[i+1]);  //LEADTEK format // Write value to flash
      *Flash_ptr =((unsigned int)(FBuf[i])|(FBuf[i+1]<<8));   //Use TI Format
    }
    Flash_ptr++;
    __delay_cycles(1000);
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void flash_erase_multi_segments (unsigned int Seg_Address,unsigned int Number_of_segments)
{
  char *Flash_Ptr;                      // Flash pointer
  unsigned int i;
  for (i=0;i<=Number_of_segments;i++){
  Flash_Ptr = (char *) (Seg_Address + 512 * i) ;     // Initialize Flash pointer
  FCTL3 = FWKEY;                        // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                // Set Erase bit
  *Flash_Ptr = 0;                       // Dummy write to erase Flash segment
  FCTL1 = FWKEY;                        // Clear Erase bit
  FCTL3 = FWKEY + LOCK;                 // Reset LOCK bit
  }
}
