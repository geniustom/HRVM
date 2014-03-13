//#define BootDebug

#ifdef BootDebug


#include "../../Driverlib/DriverExport.h"
#include "../BootloaderExport.h"

extern void Reset(void);
extern void ShowMSGtoOLED(unsigned char *SBUF,unsigned int length,unsigned char Abort);
extern UpdatePointer UpdateP;
unsigned char USBInStart,OSisRunning;



void Run_Main(){
  P2DIR &= BIT0 | BIT1 | BIT2 | BIT6;  //set SW1~3、USB_DETECTOR as input
  USBInStart=((P2IN&BIT6)!=0)? 1:0;  //USB IN: 1 else 0  
 //------------------------------------------------------------------   
    P2IE|=BIT6;      // When USB State Change Then Reset OS 
    if (USBInStart==1)P2IES|=BIT6;  //P2IES=1 interrupt at 1->0 
    else if (USBInStart==0)P2IES&=~BIT6;  //P2IES=0 interrupt at 0->1  
//------------------------------------------------------------------
  WDTCTL = WDT_ADLY_1000;               // WDT 1000ms, ACLK, interval timer
  IE1 |= WDTIE;                         // Enable WDT interrupt
  P6DIR|=BIT5;
  ShowMSGtoOLED("BOOTLOADER 2.2 ",15,0);
  while(1);
  //_BIS_SR(LPM0_bits + GIE);             // Enter LPM3 w/interrupt
}

void DoNormalRx(unsigned char RX){  //ECHO THE DATA
    while (!(IFG1 & UTXIFG0));                                   // USART0 TX buffer ready?
    TXBUF0 = RX;
}


void main( void ){
  BootLoaderMain(Run_Main,DoNormalRx);
}
/*
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR(void)   //常態1..負緣觸發ISR
{
    if ((P2IFG&BIT6)!=0){
      P2IFG = 0;
      Reset();  //若USB插或拔..就RESET
    };
    P2IFG = 0;
}
*/
// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void){
  P6OUT ^= BIT5;                        // Toggle P6.5 using exclusive-OR
}

#endif

