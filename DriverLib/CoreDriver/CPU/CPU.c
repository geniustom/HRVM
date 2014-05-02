#include "../../DriverExport.h"

void Use_XTAL2(void){
  #ifdef MSP430F16x 
    BCSCTL1 &= ~XT2OFF;                         // XT2= HF XTAL
    do{
      IFG1 &= ~OFIFG;                           // Clear OSCFault flag
      for (int i = 0xFF; i > 0; i--);           // Time for flag to set
    }while ((IFG1 & OFIFG));                    // OSCFault flag still set?
    //BCSCTL2 |= SELM_2;                        // MCLK= XT2 (safe)
    BCSCTL2 |= SELM_2+SELS;                     // MCLK=SMCLK=XT2 (safe)
  #else
    P5SEL  |= BIT2 + BIT3;                      // Port select XT2
    UCSCTL6 &= ~XT2OFF;                         // Enable XT2
    UCSCTL3 |= SELREF_2;                        // FLLref = REFO
    UCSCTL4 |= SELA__REFOCLK;                   // ACLK=REFO(32.768kHz),SMCLK=DCO,MCLK=DCO
    do{                                         // Clear XT2,XT1,DCO fault flags                                             
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);                                                    
      SFRIFG1 &= ~OFIFG;                        // Clear fault flags
    }while (SFRIFG1&OFIFG);                     // Test oscillator fault flag

    UCSCTL6 &= ~(XT2DRIVE0 + XT2DRIVE1);        // Decrease XT2 Drive according to expected frequency                                             
    UCSCTL6 |= XT2DRIVE_2;                      // XT2 Drive Level mode: 2, XT2: 16MHz~24MHz                      
    UCSCTL4 |= SELS_5 + SELM_5;                 // SMCLK=MCLK=XT2
  #endif
}


void Use_DCO(unsigned long CLK){
  //ACLK = REFO = 32kHz, MCLK = SMCLK = 25MHz  
  UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO  
  UCSCTL4 |= SELA_2;                        // Set ACLK = REFO  
  
  __bis_SR_register(SCG0);                  // Disable the FLL control loop  
  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx  
  UCSCTL1 = DCORSEL_7;                      // Select DCO range 25MHz operation  
  UCSCTL2 = (CLK/32768) + FLLD_1;                   // Set DCO Multiplier for 25MHz  
                                          // (N + 1) * FLLRef = Fdco  
                                          // (762 + 1) * 32768 = 25MHz  
                                          // Set FLL Div = fDCOCLK/2  
  __bic_SR_register(SCG0);                  // Enable the FLL control loop  
  
  // Worst-case settling time for the DCO when the DCO range bits have been  
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx  
  // UG for optimization.  
  // 32 x 32 x 25 MHz / 32,768 Hz = 782000 = MCLK cycles for DCO to settle  
  __delay_cycles(782000);  
  
  // Loop until XT1,XT2 & DCO fault flag is cleared  
  do{  
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);  
                                          // Clear XT2,XT1,DCO fault flags  
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags  
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag     
}

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
