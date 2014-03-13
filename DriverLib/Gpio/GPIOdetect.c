#include "../DriverExport.h"

GPIOHookStruct GPIOHook[MaxOfHook];
/************************************************************
                        HookType 種類
1. Lo->Hi 正緣觸發
2. Hi->Lo 負緣觸發
3. Lo->Hi 或 Hi->Lo 都觸發
************************************************************/
void SetISRtype(unsigned char GPIO,unsigned char HookType){
  P2DIR &=GPIO;  //set input
  switch (HookType){
  case 1:
    P2IES&=~GPIO;  //P2IES=0 interrupt at 0->1
    break;
  case 2:
    P2IES|=GPIO;  //P2IES=1 interrupt at 1->0 
    break;
  case 3:
    P2DIR &=GPIO;  //set input
    unsigned char HLdetect=((P2IN&GPIO)!=0)? 1:0;  //USB IN: 1 else 0  
    if (HLdetect==1)P2IES|=GPIO;  //P2IES=1 interrupt at 1->0 
    else if (HLdetect==0)P2IES&=~GPIO;  //P2IES=0 interrupt at 0->1
    break;
  }
  P2IE|=GPIO;
}

void AddGPIOHook(int GPIOID,unsigned char HookType,void (*HookCallBack)(),unsigned char HookIndex){
  GPIOHook[HookIndex].GPIOID=GPIOID;
  GPIOHook[HookIndex].HookCallBack=HookCallBack;
  GPIOHook[HookIndex].HookType=HookType;  //HOOK種類 1:正緣觸發 2:負緣觸發 3:雙向觸發 
  unsigned char GPIO=GPIOID&0xff;
  SetISRtype(GPIO,HookType);
}

void GPIODetect(){
  for(int i=0;i<MaxOfHook;i++){
    unsigned char GPIO=GPIOHook[i].GPIOID&0xff;
    if ((P2IFG&GPIO)!=0){
      if((WDTCycle-GPIOHook[i].GPIOStartTime)>100){
        GPIOHook[i].GPIOStartTime=WDTCycle; 
        SetISRtype(GPIO,GPIOHook[i].HookType);
        AddISRMessage(&ISRMSG,ISRPIRORITY_GPIOdetect,GPIOHook[i].HookCallBack,GPIO);
      }
    }
  }
  P2IFG = 0;
}
