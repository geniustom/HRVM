#include "../DriverExport.h"


BatteryStruct BattStruct;


void Compute_BattGrid(){
  //---------------------------------------X----0----1----2----3
  //const unsigned int LOBatteryThreshold[4]={2240,2302,2391,2480};  //3.3v的參考電位時
  //---------------------------------------X----0----1----2----3
  const unsigned int LOBatteryThreshold[4]={2956,3039,3156,3273};  //2.5v的參考電位時
  //-------------------------------------------------處理平均電壓
  BattStruct.BatteryArray[BattStruct.BattIndex]=AD_Struct.ADCBUF[1];
  BattStruct.BattIndex++;
  BattStruct.BattIndex%=BatteryArraySize;
  if(BattStruct.BattIndex!=0)return; //每當平均完一輪才計算一次電量
  unsigned int AVG=0;
  for(int i=0;i<BatteryArraySize;i++){AVG+=BattStruct.BatteryArray[i];}
  AVG/=BatteryArraySize;
  //-------------------------------------------------處理平均格數
  unsigned char BatteryGrid=0;//計算目前格數
  for(unsigned char i=0;i<4;i++){if(AVG>=LOBatteryThreshold[i]){BatteryGrid++;}};
  BattStruct.BatteryGridArray[BattStruct.BattGridIndex]=BatteryGrid;
  BattStruct.BattGridIndex++;
  BattStruct.BattGridIndex%=BatteryGridArraySize;
  if(BattStruct.InitCount<BatteryGridArraySize){BattStruct.InitCount++;}
  //------------------------------------------------計算目前電池格數
  if(BattStruct.InitCount<BatteryGridArraySize){
    BattStruct.BattGrid=BatteryGrid;
  }else{
    unsigned char AvgGrid;
    for(int i=0;i<BatteryGridArraySize;i++){   //要連續10個都一樣才允許電池格數變動
      AvgGrid+=BattStruct.BatteryGridArray[i];
    }
    if(AvgGrid%BatteryGridArraySize==0)BattStruct.BattGrid=BatteryGrid; 
    //都一樣才允許變動(用取魚數的方式來判定就知道了)
  }
}

void Open_BattDetect(){
  BattStruct.InitCount=0;
  BattStruct.BattDetectState=1;
  BattStruct.BattGridIndex=0;
  for(int i=0;i<BatteryGridArraySize;i++)BattStruct.BatteryGridArray[i]=1;
  BattStruct.BattGrid=1;
  TurnOnADC();
  
  BattStruct.BattID=TakeTimer(RANDOM,32768/32,0,Compute_BattGrid);
}

void Close_BattDetect(){
  BattStruct.BattDetectState=0;
  Close_ADC();
  ReleaseTimer(BattStruct.BattID);
}

#ifdef MSP430F16x 
void Use_DCO(void)
{
 BCSCTL2 = 0;  //use DCO
 BCSCTL1 |= XT2OFF;                   // XT2= HF XTAL
}
#endif


#ifdef MSP430F16x 
void Use_XTAL2(void)
{
  BCSCTL1 &= ~XT2OFF;                   // XT2= HF XTAL
  do
  {
  IFG1 &= ~OFIFG;                       // Clear OSCFault flag
  for (int i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG));               // OSCFault flag still set?
  BCSCTL2 |= SELM_2;                    // MCLK= XT2 (safe)
}
#endif

void Reset(){
  asm("MOV 0xFFFE,PC");
}

void PowerDown(){
  P6DIR|=BIT4;
  P6OUT|=BIT4;
  //Dis_OLED();
  //StopADC();
/* 
  P1DIR = 0xFF;                         // All P1.x outputs
  P1OUT = 0;                            // All P1.x reset
  //P2DIR = 0xFF;                         // All P2.x outputs
  P2OUT = 0;                            // All P2.x reset
  P3DIR = 0xFF;                         // All P3.x outputs
  P3OUT = 0;                            // All P3.x reset
  P4DIR = 0xFF;                         // All P4.x outputs
  P4OUT = 0;                            // All P4.x reset
  P5DIR = 0xFF;                         // All P5.x outputs
  P5OUT = 0;                            // All P5.x reset
  P6DIR = 0xFF;                         // All P6.x outputs
  P6OUT = 0;                            // All P6.x reset
*/
  
  //P5OUT &= ~ECGShutdown;
  //P6OUT &= ~BeepPort;
  //NAND_Close();
  //Use_DCO();
  //_BIS_SR(LPM3_bits + GIE);           // Enter LPM3
}


void PowerDownLongTime(){
  P6DIR|=BIT4;
  for(long i=0;i<8000;i++)_NOP();
  P6OUT|=BIT4;
}

void PowerOn(){
  Use_XTAL2();
  En_OLED();
}
