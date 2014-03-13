#include "../DriverLib/DriverExport.h"        // Standard Equations
#include "RtosExport.h"

void Init_Device(void (*MainTaskFunctionPoint)()){
  if(MainTaskFunctionPoint!=0x00)MainTaskFunctionPoint();
}

void Init_RTOS(){
  InitWDT();
  ShowErrorCode();
}

void Run_RTOS(void (*BeforeSCH)(),unsigned char RunDelayTaskB,void (*AfterSCH)(),unsigned char RunDelayTaskA){
  unsigned char RTOS_RunDelayTaskB,RTOS_RunDelayTaskA;
  _EINT();
  while(1){
    RTOS_RunDelayTaskB++;
    RTOS_RunDelayTaskA++;
    RTOS_RunDelayTaskA%=RunDelayTaskA;
    RTOS_RunDelayTaskB%=RunDelayTaskB;
    if(BeforeSCH!=0x00&&RTOS_RunDelayTaskB==0)BeforeSCH();
    PQSched(&TaskPQ);
    if(AfterSCH!=0x00&&RTOS_RunDelayTaskA==0)AfterSCH();
    //LPM0;           // Enter LPM3
  };
}


