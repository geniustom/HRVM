#define Debug


#ifdef Debug

#include "DriverExport.h"


void Init_CLK(void)
{
    WDTCTL = WDTPW | WDTHOLD;                // Stop WDT
 /*
    BCSCTL1 &= ~XT2OFF; // XT2= HF XTAL &= ~XT2OFF;
    do{
      IFG1 &= ~OFIFG; // Clear OSCFault flag
      for (int i = 0xFF; i > 0; i--); // Time for flag to set
    }while ((IFG1 & OFIFG)); // OSCFault flag still set? 
    BCSCTL2 |= SELM_2+SELS; // MCLK=SMCLK=XT2 (safe)
*/
    //Use_DCO(25000000);
    Use_XTAL2();
}



void main(){
  Init_CLK();
  SDCard_init();
  while(1){
    WriteTest();
  }
  //ContinueWriteTest();
}



















/*
#include <Math.h>
#include <Stdlib.h>

#define MAX(a, b) (a > b ? a : b)
extern void BootLoaderMain(void(*Run_MainFunct)(void),void(*DoNormalRxFunct)(unsigned char RxData));

unsigned char USB[512];
unsigned char USB1[512];
unsigned long nAddr;
unsigned int MaxERRbytePerPage=0;
unsigned char FD,FC;
unsigned char Data=0;
unsigned long ErrCount=0;
unsigned int ACOL,MaxThisPage=0;
FlashStruct FS;


void main(){
  Init_CLK();
  Init_Flash();
  _EINT(); 	
  P6DIR|=BIT5;
  for(unsigned long i=0;i<131072;i++){
    nAddr = i; // 擦除塊地址
    MaxThisPage=0;
    FS.AccessPage=nAddr;
    for(int j = 0;j < 512;j++){
      USB[j]=1+(rand()%254);
      USB1[j]=USB[j];
    }
    FSave_Data(nAddr,USB);
    for(int j = 0;j < 512;j++)USB[j]=0;
    FRead_Data(nAddr,USB);
    
    for(int j = 0;j < 512;j++){
      ACOL=j;
      if(USB[j]!=USB1[j]){ 
        ErrCount++;
        MaxThisPage++;
      }
    }
    MaxERRbytePerPage=MAX(MaxERRbytePerPage,MaxThisPage);
  }
}
*/

/*
void main(){
  Init_CLK();
  Init_Flash();
  _EINT(); 	
  P6DIR|=BIT5;
  InitFlashStruct(&FS,USB,0,0);

//  for(unsigned long i=0;i<131072;i++){
//    nAddr = i; // 擦除塊地址
//    BlockErase(nAddr,1);
//  }

  for(unsigned long i=0;i<131072;i++){
    nAddr = i; // 擦除塊地址
    MaxThisPage=0;
    FS.AccessPage=nAddr;
    FS.NeedtoAccess=2;
    for(int j = 0;j < 512;j++){
      USB[j]=1+(rand()%254);
      USB1[j]=USB[j];
    }
    //FSave_Data(nAddr,USB);
    for(int j = 0;j < 5000;j++){
      StepSave_Flash(&FS);
      P6OUT&=~BIT5;
    }
    for(int j = 0;j < 512;j++)USB[j]=0;
    //FRead_Data(nAddr,USB);
    FS.NeedtoAccess=1;
    
    for(int j = 0;j < 2000;j++){
      StepRead_Flash(&FS);
    }
    
    for(int j = 0;j < 512;j++){
      ACOL=j;
      if(USB[j]!=USB1[j]){
        ErrCount++;
        MaxThisPage++;
      }
    }

    MaxERRbytePerPage=MAX(MaxERRbytePerPage,MaxThisPage);
  }
}
*/


/*
void main(void){ 
  WDTCTL = WDTPW | WDTHOLD;                // Stop WDT
    
  BCSCTL1 &= ~XT2OFF; // XT2= HF XTAL &= ~XT2OFF;
  do{
    IFG1 &= ~OFIFG; // Clear OSCFault flag
    for (int i = 0xFF; i > 0; i--); // Time for flag to set
  }while ((IFG1 & OFIFG)); // OSCFault flag still set? 
  BCSCTL2 |= SELM_2+SELS; // MCLK=SMCLK=XT2 (safe)
  
 
  int x=0;
  double y=0;  
  Init_OLED();
  En_OLED();
  CreateChart(0,16,127,47,512,256,&OLC);
  P6DIR|=BIT5;

  
  while(1){
    y=floor(cos((float)x/180)*128);
    AddValue(&OLC,floor(y));
    x+=4;
    ShowPIC1Page();
  }
 
}

void main( void ){
  WDTCTL = WDTPW | WDTHOLD;                // Stop WDT
    
  BCSCTL1 &= ~XT2OFF; // XT2= HF XTAL &= ~XT2OFF;
  do{
    IFG1 &= ~OFIFG; // Clear OSCFault flag
    for (int i = 0xFF; i > 0; i--); // Time for flag to set
  }while ((IFG1 & OFIFG)); // OSCFault flag still set? 
  BCSCTL2 |= SELM_2+SELS; // MCLK=SMCLK=XT2 (safe)
  
  Init_Devicelib();

  P2SetButtonIO(&ButtonLeft,1,BIT1,BIT1,ButtonPress,ButtonLongPress);
  P2SetButtonIO(&ButtonRight,1,BIT2,BIT2,ButtonPress,ButtonLongPress);
  P2SetButtonIO(&ButtonEnter,1,BIT0,BIT0,ButtonPress,ButtonLongPress);

  P5OUT |= BIT4 | BIT5; 
  Init_DAC();
  Close_ADC();
  Open_ADC(TestADCISR);
  CreateChart(0,16,127,47,512,256,&OLC);			//creat picture boundary
  
  _EINT();
  //ShortBeepSetTimes(3);
  //Beep1Sec();
  //int x=0,y=0;
  //ShortBeepSetTimes(200);
  while(1){
    //y=floor(cos((float)x/180)*128);
    //AddValue(&OLC,y);
    if(AD_Struct.ADIndex%256==0){Beep1Sec();}//ShortBeepSetTimes(1);}
    if(AD_Struct.ADComplete==1){
      AddValue(&OLC,128-((AD_Struct.ADCBUF[2]>>4)&0xff));
      show_pic();
      AD_Struct.ADComplete=0;
    }
    //if(x%360==0){Beep1Sec();}//BuzzerOff();}
  }
}




void ButtonPress(){
      Dis_OLED();
}

void ButtonLongPress(){
      Init_OLED();
      En_OLED();
}


void TestADCISR(){
  if(AD_Struct.ADIndex%256==0){Beep1Sec();}
}


*/

#endif



