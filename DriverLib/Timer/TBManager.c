#include "../DriverExport.h"        // Standard Equations

typedef struct{
    unsigned int  Period;
    unsigned char  Operated_Mode;
    void  (*Timer_Function_Pointer)(void);   //function pointer will be called when timeout
}TIMER_ARRAY;

TIMER_ARRAY TimerB_Array[TIMERB_AMOUNT];
unsigned int TimerB_Error=0;

void Init_TB()
{
    TBCTL = TBCLR + TBSSEL_1 + MC_2 + TBIE; //ACLK, CONTINUE mode
    for(int i=0 ; i<TIMERB_AMOUNT ; i++) TimerB_Array[i].Timer_Function_Pointer = 0;
}

TIMERID TakeTimer(TIMERID RequiredTimer,unsigned int Interval,unsigned char mode,void (*f)())
//if RequiredTimer = 0, timer is selected by ID
{
    unsigned int i,*pTBCCR,*pTBCCTL;
    if (RequiredTimer == RANDOM){
        for (i=TB2 ; i<TIMERB_AMOUNT ; i++){  //TB0 and TB1 is usd for ADC; 
            RequiredTimer = i;
            if (TimerB_Array[i].Timer_Function_Pointer == 0) break;
        }
        TimerB_Error |= ERROR_CREAT_TIMER;
    }else{
        if (TimerB_Array[i].Timer_Function_Pointer != 0){
            TimerB_Error |= ERROR_CREAT_TIMER;
            return NO_SERV;
        }
    }
    _DINT();
    TimerB_Array[RequiredTimer].Period = Interval;
    TimerB_Array[RequiredTimer].Operated_Mode = mode;
    TimerB_Array[RequiredTimer].Timer_Function_Pointer = f;
    _EINT();
    pTBCCR = (unsigned int *)(&TBCCR0 + RequiredTimer);
    pTBCCTL = (unsigned int *)(&TBCCTL0 + RequiredTimer);

    *pTBCCR = TBR + Interval;
    *pTBCCTL = CCIE;

    return RequiredTimer;
}

unsigned char ReleaseTimer(TIMERID RequiredTimer)
{
    unsigned int *pTBCCTL;
    if (RequiredTimer < TIMERB_AMOUNT){
      _DINT();                                      //enter the critical section for preventing the timer
      pTBCCTL = (unsigned int *)(&TBCCTL0 + RequiredTimer);  //ISR call null function pointer
      *pTBCCTL &= ~CCIE;
      TimerB_Array[RequiredTimer].Timer_Function_Pointer = 0;
      _EINT();
      return 1;
    }else{
      TimerB_Error |= ERROR_DELETE_TIMER;
      return 0;
    }
}

void AdjustTimer(TIMERID num,unsigned int clk)
{
   unsigned int *pTBCCR;  
   if (TimerB_Array[num].Timer_Function_Pointer != 0){
      pTBCCR = (unsigned int *)(&TBCCR0 + num);
      *pTBCCR += clk;
   }
}

void  TimerISRFunction(TIMERID num)
{
   unsigned int *pTBCCR;
   if ( num < 7 && TimerB_Array[num].Timer_Function_Pointer != 0){
      pTBCCR = (unsigned int *)(&TBCCR0 + num);
      *pTBCCR += TimerB_Array[num].Period;
      TimerB_Array[num].Timer_Function_Pointer();
      if ((TimerB_Array[num].Operated_Mode & SINGLE) != 0) ReleaseTimer(num);
   }
}


#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B0(void){
    TimerISRFunction(0);
}

#pragma vector=TIMERB1_VECTOR
__interrupt void Timer_B1(void){
    TimerISRFunction(TBIV >> 1);
}
