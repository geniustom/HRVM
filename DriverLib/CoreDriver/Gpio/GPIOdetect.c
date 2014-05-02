#include "../../DriverExport.h"


#define HOOK_PxDIR      P1DIR
#define HOOK_PxIES      P1IES
#define HOOK_PxIFG      P1IFG
#define HOOK_PxIN       P1IN


GPIOHookStruct GPIOHook[MaxOfHook];
/************************************************************
                        HookType ����
1. Lo->Hi ���tĲ�o
2. Hi->Lo �t�tĲ�o
3. Lo->Hi �� Hi->Lo ��Ĳ�o
************************************************************/
void SetISRtype(unsigned char GPIO,unsigned char HookType){
  HOOK_PxDIR &=GPIO;  //set input
  switch (HookType){
  case 1:
    HOOK_PxIES&=~GPIO;  //HOOK_PxIES=0 interrupt at 0->1
    break;
  case 2:
    HOOK_PxIES|=GPIO;  //HOOK_PxIES=1 interrupt at 1->0 
    break;
  case 3:
    HOOK_PxDIR &=GPIO;  //set input
    unsigned char HLdetect=((HOOK_PxIN&GPIO)!=0)? 1:0;  //USB IN: 1 else 0  
    if (HLdetect==1)HOOK_PxIES|=GPIO;  //HOOK_PxIES=1 interrupt at 1->0 
    else if (HLdetect==0)HOOK_PxIES&=~GPIO;  //HOOK_PxIES=0 interrupt at 0->1
    break;
  }
  P2IE|=GPIO;
}

void AddGPIOHook(int GPIOID,unsigned char HookType,void (*HookCallBack)(),unsigned char HookIndex){
  GPIOHook[HookIndex].GPIOID=GPIOID;
  GPIOHook[HookIndex].HookCallBack=HookCallBack;
  GPIOHook[HookIndex].HookType=HookType;  //HOOK���� 1:���tĲ�o 2:�t�tĲ�o 3:���VĲ�o 
  unsigned char GPIO=GPIOID&0xff;
  SetISRtype(GPIO,HookType);
}

void GPIODetect(){
  for(int i=0;i<MaxOfHook;i++){
    unsigned char GPIO=GPIOHook[i].GPIOID&0xff;
    if ((HOOK_PxIFG&GPIO)!=0){
      if((WDTCycle-GPIOHook[i].GPIOStartTime)>100){
        GPIOHook[i].GPIOStartTime=WDTCycle; 
        SetISRtype(GPIO,GPIOHook[i].HookType);
        AddISRMessage(&ISRMSG,ISRPIRORITY_GPIOdetect,GPIOHook[i].HookCallBack,GPIO);
      }
    }
  }
  HOOK_PxIFG = 0;
}
