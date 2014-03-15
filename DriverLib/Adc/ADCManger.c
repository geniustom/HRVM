#include "../DriverExport.h"

ADC_Struct AD_Struct;
ADC_Queue ADQueue_ECG;

void SetADCCTL(){ 
  ADC12CTL0 = ADC12ON       |             //開啟ADC核心
              SHT0_1        |             //設定取樣計時器的除頻 0:4 1:8 2:16 3:32 4:64 5:128 6:192 7:256 8:384 9:512 10:768 11:1024
              REF2_5V       |             //參考電壓2.5V
              MSC           |             //啟動多重取樣轉換模式
              REFON;                      //開啟參考電壓(不用時要關掉比較省電)
  
  
  ADC12CTL1 = CONSEQ_3      |             //選擇通道轉換方式(順序通道轉換)
              SHS_0         |             //取樣與保持的來源觸發訊號 0:ADC12SC 1:TA1 2:TB0 3:TB1
              SHP           |             //0:直接輸入外部取樣的CLK  1:經取樣計時器，為脈衝取樣模式
              ADC12DIV_2    |             //對ADC12CLK做除頻 1~8x
              ADC12SSEL_1;                //ADC12CLK來源 0:ADC12OSC 1:ACLK 2:MCLK 3:SMCLK
                
  
  ADC12MCTL0 = INCH_0 | SREF_1;           //2.5 vref
  ADC12MCTL1 = INCH_1 | SREF_1;           //2.5 vref
  ADC12MCTL2 = INCH_2 | SREF_1 | EOS;     //2.5 vref
 
  
  P6SEL   |=  BIT0 | BIT1 | BIT2;
  ADC12IE |=  BIT0 | BIT1 | BIT2; 

}

void Close_ADC(){
  AD_Struct.TurnON=0;
  ReleaseTimer(AD_Struct.TimerID);
  
  ADC12CTL0 &= ~ENC;
  
  ADC12CTL0 &=  ~(ADC12ON | REFON);
  P6SEL     &=  ~(BIT0 | BIT1 | BIT2);
  ADC12IE   &=  ~(BIT0 | BIT1 | BIT2);  
  
  //Close_TA(TAADC);
}

unsigned int GetDataformADQueue(unsigned char ChanelID){
  _DINT();
  unsigned char Rear=AD_Struct.ADQueue.Rear;
  unsigned int Result=AD_Struct.ADQueue.ADQueueData[Rear];
  if (AD_Struct.ADQueue.Front==AD_Struct.ADQueue.Rear){
    Result=AD_Struct.ADCBUF[AD_Struct.NowCapChanel];
    _EINT();
    return Result;
  }
  AD_Struct.ADQueue.Rear=(Rear+1)%ADQueueSize;
  _EINT();
  return Result;
}


//unsigned int TestADC=0;  //測試ADC是否正常用

void ADCISR(){
  unsigned char Front=AD_Struct.ADQueue.Front;
  _DINT();
  AD_Struct.ADQueue.ADQueueData[Front]=AD_Struct.ADCBUF[AD_Struct.NowCapChanel];
/**********************測試ADC是否正常用**********************
  TestADC+=32;
  TestADC&=0xfff;
  AD_Struct.ADQueue.ADQueueData[Front]=TestADC;
*************************************************************/  
  AD_Struct.ADQueue.Front=(Front+1)%ADQueueSize;
  AD_Struct.TurnON=1;
  AD_Struct.ADIndex++;
  AD_Struct.ADComplete=1;
  AddISRMessage(&ISRMSG,ISRPIRORITY_ADC,AD_Struct.CallBackFP,ISRMSG_ADC);
  if(AD_Struct.ADQueue.Front==AD_Struct.ADQueue.Rear){
    AddErrorCode(ADCQueueFull); //表示OS處理不及..掉了SAMPLE
  } //full overwrite data
  _EINT();
}

void TurnOnADC(){
  ADC12CTL0 &= ~ENC;
  
  SetADCCTL();
  
  ADC12CTL0 |= ENC;
  ADC12CTL0 |= ADC12SC;
}

void Open_ADC(void (*CallBackFP)(),unsigned char CapChanel){
  AD_Struct.CallBackFP=CallBackFP;  //Call back function point..trigger at TA0 ISR
  
  TurnOnADC();
  
  AD_Struct.NowCapChanel=CapChanel;
  AD_Struct.ADQueue.Front=0;
  AD_Struct.ADQueue.Rear=0;
  AD_Struct.TimerID=TakeTimer(TBADC,(unsigned int)(32768/256),0,ADCISR);
  //Open_TA(TAADC);
}

#ifdef MSP430F16x 
void Init_DAC(){
  // 輸出1V DAC模擬電壓
  P6DIR |= BIT6;
  DAC12_0CTL = DAC12IR + DAC12AMP_5 + DAC12ENC;
  DAC12_0DAT = 0x0666;
}
#endif

#pragma vector=ADC_VECTOR
volatile __interrupt void ADC12ISR (void){
  AD_Struct.ADCBUF[0]=ADC12MEM0;
  AD_Struct.ADCBUF[1]=ADC12MEM1;
  AD_Struct.ADCBUF[2]=ADC12MEM2;
  //ADCBUF[3]=ADC12MEM3;
  //ADCBUF[4]=ADC12MEM4;
  //ADCBUF[5]=ADC12MEM5;
  //ADCBUF[6]=ADC12MEM6;
  //ADCBUF[7]=ADC12MEM7;
}

