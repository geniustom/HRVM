//#define RTOSDebug
#ifdef RTOSDebug  

#include "../DriverLib/DriverExport.h"        // Standard Equations
#include "Kernel/Pirority_Q.h"
#include"RtosExport.h"
#include "App/MenuUI.h"


extern void Reset(void);
extern void ShowMSGtoOLED(unsigned char *SBUF,unsigned int length,unsigned char Abort);
extern void BootLoaderMain(void(*Run_MainFunct)(void),void(*DoNormalRxFunct)(unsigned char RxData));
extern void ButtonPress();
extern void ButtonLongPress();

void Init_Button(){
  P2SetButtonIO(&ButtonLeft,1,ButtonL,ButtonL,ButtonPress,ButtonLongPress);
  P2SetButtonIO(&ButtonRight,1,ButtonR,ButtonR,ButtonPress,ButtonLongPress);
  P2SetButtonIO(&ButtonEnter,1,ButtonC,ButtonC,ButtonPress,ButtonLongPress);  
}

void Run_Main(){
  Init_Devicelib();
  
  Init_Device(Init_Button);
  Init_RTOS();
  P6DIR|=BIT5;
  //ShowMSGtoOLED("GENIUSTOM",9,0);
  Init_RTC();
  Start_RTC(Show_DateTime);
  CurrentNode=Main_DateTime;
  Run_RTOS(0,show_pic);
}

void DoNormalRx(unsigned char RX){  //ECHO THE DATA
    while (!(IFG1 & UTXIFG0));                                   // USART0 TX buffer ready?
    TXBUF0 = RX;
}




void main( void ){
  //BootLoaderMain(Run_Main,DoNormalRx);
  //------------------¯ÂOS------------------------
      WDTCTL = WDTPW | WDTHOLD;                // Stop WDT
    
    BCSCTL1 &= ~XT2OFF; // XT2= HF XTAL &= ~XT2OFF;
    do{
      IFG1 &= ~OFIFG; // Clear OSCFault flag
      for (int i = 0xFF; i > 0; i--); // Time for flag to set
    }while ((IFG1 & OFIFG)); // OSCFault flag still set? 
    BCSCTL2 |= SELM_2+SELS; // MCLK=SMCLK=XT2 (safe)

    Run_Main();
  //---------------------------------------------
}



#endif


