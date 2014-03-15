#include "../../DriverExport.h"


#ifdef MSP430F16x 
void Use_DCO(void){
  BCSCTL2 = 0;  //use DCO
  BCSCTL1 |= XT2OFF;                   // XT2= HF XTAL
}
#endif


#ifdef MSP430F16x 
void Use_XTAL2(void){
  BCSCTL1 &= ~XT2OFF;                   // XT2= HF XTAL
  do{
    IFG1 &= ~OFIFG;                       // Clear OSCFault flag
    for (int i = 0xFF; i > 0; i--);           // Time for flag to set
  }while ((IFG1 & OFIFG));               // OSCFault flag still set?
  BCSCTL2 |= SELM_2;                    // MCLK= XT2 (safe)
}
#endif


void ResetMCUByPC(){
   asm("MOV 0xFFFE,PC");
}

void ResetMCUByPMM(){
  PMMCTL0 = PMMPW + PMMSWBOR + (PMMCTL0 & 0x0003); 
}

void ResetMCUByWDT(){
  WDTCTL = WDT_MRST_0_064; 
  while(1);
}
