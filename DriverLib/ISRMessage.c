#include "DriverExport.h"

ISRMessageQueue ISRMSG;

volatile void Init_Devicelib(){
  Init_OLED();
  En_OLED();
  Init_TA();
  Init_TB();
  ClearISRMessage(&ISRMSG);
}

void Open_TA(unsigned char ID){
  switch (ID){
    case 0:
      TACCR0=TAR + TACCR_0; 
      TACCTL0 = CCIE;  
      break;
    case 1:
      TACCR1=TAR + TACCR_1; 
      TACCTL1 = CCIE;  
      break;
    case 2:
      TACCR2=TAR + TACCR_2; 
      TACCTL2 = CCIE;  
      break;
  }
  //NOTE:�����TACCTLX|=CCIE �_�h�|�v�T��O�ժ�COUNT
}


void Close_TA(unsigned char ID){
  switch (ID){
    case 0:
      TACCTL0 &= ~CCIE;  
      break;
    case 1:
      TACCTL1 &= ~CCIE;  
      break;
    case 2:
      TACCTL2 &= ~CCIE;  
      break;
  }
}

void Init_TA(){
  TACTL = TACLR + TASSEL_1 + MC_2 + TAIE;
  TACCR0=TACCR_0;
  TACCR1=TACCR_1;
  TACCR2=TACCR_2;
}


void AddISRMessage(ISRMessageQueue *IM,unsigned char PirorityValue,void (*F)(),unsigned char MSGType){
  if (IM->Index>=MSGQueueSize){
    AddErrorCode(ISRQueueFull); //OVERFLOW ISR QUEUE FULL
    //Reset();
  }
  _DINT();
  IM->MSGNode[IM->Index].PirorityValue=PirorityValue;
  IM->MSGNode[IM->Index].FunctionAddr=F;
  IM->MSGNode[IM->Index].MSGType=MSGType;
  IM->Index++;
  _EINT();
}

void ClearISRMessage(ISRMessageQueue *IM){
    IM->Index=0;
}


#pragma vector = PORT2_VECTOR
__interrupt void P2ISR(void){   //�`�A1..�t�tĲ�oISR
  ButtonDetect();
  GPIODetect();
  LPM3_EXIT;
  P2IFG = 0;  //�M��FLAG..���M�|�����_
}


#pragma vector = TIMERA0_VECTOR
__interrupt void Timer_A0(void){  //--------------FOR RTC USE
    P6OUT^=BuzzerPin;
    TACCR0=TAR+TACCR_0;
}


#pragma vector = TIMERA1_VECTOR
__interrupt void Timer_A3(void){
  switch( TAIV ){
    case 2: 
      UpdateMsec();
      TACCR1=TAR+TACCR_1;
      break;
    case 4:
      CheckButton(&ButtonLeft);
      CheckButton(&ButtonRight);
      CheckButton(&ButtonEnter);
      TACCR2=TAR+TACCR_2;
      break;
  }
}

/*
#pragma vector = PORT1_VECTOR
__interrupt void P1ISR(void){
  P1IFG = 0;
}
*/

