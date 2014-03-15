#include "../../DriverExport.h"

unsigned long WDTCycle;

void InitWDT(){
  WDTCTL = WDT_ADLY_1_9;
  IE1 |= WDTIE;                         // Enable WDT interrupt
}


// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void){
  WDTCycle++;
}

