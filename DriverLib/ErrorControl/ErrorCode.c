#include "../DriverExport.h"

unsigned int ERRORCODE=0;

void AddErrorCode(unsigned int ERRORTYPE){
  ERRORCODE|=ERRORTYPE;
  ShowErrorCode();
  //Reset();
}


void ShowErrorCode(){
  Init_OLED();
  En_OLED();
  ClearScreen();
  unsigned char *ERRMSG;
  switch(ERRORCODE){
  case ISRQueueFull:
    ERRMSG="ISR QUEUE FULL ";
    break;
  case TaskQueueFull:
    ERRMSG="TASK QUEUE FULL";
    break;
  case ADCQueueFull:
    ERRMSG="ADC QUEUE FULL ";
    break;
  case FlashBufFull:
    ERRMSG="FLASH BUF FULL ";
    break;    
  case InternalFlashErr:
    ERRMSG="INTER FLASH ERR";
    break;        
  }
  if(ERRORCODE!=0){
    _DINT();
    while(1){
      Print(ERRMSG,15,2,2,8);
      show_pic();
    }
  }
}

