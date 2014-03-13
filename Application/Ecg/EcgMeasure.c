#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"
#include "../Menu/MenuUI.h"


const char OLED_SHOW_Small_HR[32]={                     // HR Image
  0x70,0xF8,0xFC,0xFC,0xFC,0xF8,0xF0,0xE0,0xE0,0xF0,0xF8,0xFC,0xFC,0xFC,0xF8,0x70,
  0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00
};
const int HP_Point33Hz[64]={1,3,6,11,20,32,49,72,103,142,190,249,319,402,498,606,727,859,1002,1154,1312,1475,1639,1802,1960,2110,2247,2370,2475,2560,2622,2660,2672,2660,2622,2560,2475,2370,2247,2110,1960,1802,1639,1475,1312,1154,1002,859,727,606,498,402,319,249,190,142,103,72,49,32,20,11,6,3};
const int LP_40Hz[64]={37,-67,-142,-88,84,228,173,-85,-334,-303,55,454,487,23,-585,-743,-175,716,1092,437,-842,-1590,-891,949,2364,1733,-1035,-3885,-3769,1087,9454,17409,20673,17409,9454,1087,-3769,-3885,-1035,1733,2364,949,-891,-1590,-842,437,1092,716,-175,-743,-585,23,487,454,55,-303,-334,-85,173,228,84,-88,-142,-67};
FlashStruct ECGRawFile;

//ECGStructure ORGECG;
//int ORGBUF[64];
//ECGStructure BaselineECG;
//int BaselineBUF[64];
ECGStructure LP40ECG;
int LP40BUF[64];
CircultBuffer8BIT DiffECG;
unsigned char DiffBUF[64];
unsigned char ECGRawBackBuf[ECGRawBackBufLen];
ECGData ECGStruct;
/// show Heart rate on OLED display ///
void show_HR(unsigned int HR){
  int HR_Div_100;
  Full_Block(0,0,48,16,0x00);  
  if((HR>ECG_range_max)||(HR<ECG_range_min)){HR=0;}
  ECGStruct.NoHRDelay++;
  if(ECGStruct.NoHRDelay>3){  ECGStruct.ShowHRGraph=0;  }
  if(ECGStruct.NoHRDelay>10){  ECGStruct.AVGHR=0; }
  
  if (HR>0)  {
    if(ECGStruct.ShowHRGraph==1){DrawBlock_Area(0,0,16,16,(unsigned char *)OLED_SHOW_Small_HR);}
    else{Full_Block(0,0,16,16,0x00);} 
    
    HR_Div_100 = HR / 100;
    DrawBlock(20,4,8,8,(unsigned char *)OLED_Size8_Dig[HR_Div_100]);
    DrawBlock(30,4,8,8,(unsigned char *)OLED_Size8_Dig[(HR - (HR_Div_100 * 100)) / 10]);
    DrawBlock(40,4,8,8,(unsigned char *)OLED_Size8_Dig[(HR % 10)]);
    
  }  
  else
  {
    //Close_TA1();
    DrawBlock_Area(0,0,16,16,(unsigned char *)OLED_SHOW_Small_HR);       
    DrawBlock(20,4,8,8,(unsigned char *)OLED_Size8_Dig[11]);
    DrawBlock(30,4,8,8,(unsigned char *)OLED_Size8_Dig[11]);
    DrawBlock(40,4,8,8,(unsigned char *)OLED_Size8_Dig[11]);
  }

  //RTC_Show_Test_Time();    
}

void RTC_Show_Test_Time(){
  Full_Block(67,0,60,16,0x00);
  
  unsigned char K[11]={'0','1','2','3','4','5','6','7','8','9',':'};    
  // Show Time
  
  unsigned int TestTime=(ECGMeasureBound-ECGStruct.MeasureTime)>>8;
  unsigned char ECGMin=TestTime/60;
  unsigned char ECGSec=TestTime%60;
  if (ECGMin < 5){
     Print(&K[ECGMin % 10],1,60,4,8);				
     Print(&K[10],1,70,4,8);				//:
     Print(&K[ECGSec / 10],1,80,4,8);				
     Print(&K[ECGSec % 10],1,90,4,8);		
   }
  /*/---------------------------------------------------------Show Gain
   const unsigned char X[8]={0x00,0xC6,0xEE,0x18,0x30,0xEE,0xC6,0x00};
   DrawBlock(110,4,8,8,(unsigned char *)X);
   Print(&K[ECGStruct.DigitalGain],1,119,4,8);
  //----------------------------------------------------------------*/
}



void ComputeDigitalGain(){
  int MaxV=0;
  for(int i=0;i<64;i++){MaxV=max(MaxV,LP40BUF[i]);}
  int OffsetMaxV=MaxV-2048;
  if ((OffsetMaxV>=50)&&(OffsetMaxV<=200)){
    ECGStruct.DigitalGain=6;
  }else if ((OffsetMaxV>200)&&(OffsetMaxV<=500)){
    ECGStruct.DigitalGain=4;
  }else if ((OffsetMaxV>500)&&(OffsetMaxV<=1000)){
    ECGStruct.DigitalGain=2;
  }else{ECGStruct.DigitalGain=1;}
}


void InitECGStruct(){
  InitSDNNData();
  ECGStruct.MeasureTime=0;
  ECGStruct.RRIV=0; 
  ECGStruct.ECG_index=0;
  //ECGStruct.HRAvgCount=0;
  ECGStruct.NoHRSampleDelay=0;
  ECGStruct.AVGHR=0;
  for(int i=0;i<10;i++)ECGStruct.HRArray[i]=0;
  for(int i=0;i<20;i++)ECGStruct.Smooth[i]=100;
  ECGStruct.HRAvgIndex=0;
  //ECGStruct.TempBufIndex=0;
  ECGStruct.DigitalGain=1;
  ECGStruct.NoHRDelay=0;
  InitFlashStruct(&ECGRawFile,USB,ECGRawBackBuf,(unsigned int)ECGRawBackBufLen);
}



void StopECG(){       
  P5OUT &= ~BIT4;
  AD_Struct.ADComplete=0;
  Close_ADC();
  //ReleaseCoefBuffer(&ORGECG.CBUF);  
  //ReleaseCoefBuffer(&BaselineECG.CBUF); 
  //ReleaseCoefBuffer(&RRIVSquence);
  //ReleaseCoefBuffer8BIT(&DiffECG);
  //ReleaseCoefBuffer8BIT(&MAVariant);

}

//量測時間結束時執行
void Test_TimeOut(unsigned char Check_Success){
//正常 MODE下-------------------------------------------------------------
    CurrentNode=ECG_Success;
    Draw_BG_Image();
//------------------------------------------------------------------------  
  ShortBeepSetTimes(1); //短逼1聲..有效操作
  if(WhenExitMenuNode!=0)WhenExitMenuNode();
    if(CurrentNode.OnEnterNode!=0x00)CurrentNode.OnEnterNode(); //若有進入節點所引發的事件..從這執行
      WhenExitMenuNode=CurrentNode.OnExitNode;
}


void SaveRRIV(){
  unsigned long Addr=Find_Start(UserDataStruct.EntryUser)+192;
  BlockErase(Addr,1);
  for(int i=0;i<5;i++){
    for(int j=0;j<256;j++){
      unsigned int RRValue=RRIVmsSaveFlash[i*256+j];
      USB[j*2]=(unsigned char)(RRValue>>8);
      USB[j*2+1]=(unsigned char)(RRValue&0xff);
    }
    FSave_Data(Addr,USB);
    Addr++;
  }
}

void ECGFinal(){
  SaveRRIV();
  Save_UserIndex(UserDataStruct.EntryUser);
  Test_TimeOut(0x00);               //CurrentNode=ECG_Success;  
  ECGStruct.ECGFinal=0;
   
  PrintSDNN();
  InitECGStruct();
  SetDisCount(AutoShutdownTimeout,Reset);   //按鍵後閑置6分鐘之後RESET(自動關機)
}

void MeasureComplete(){
    if(ECGStruct.MeasureTime>=ECGMeasureBound){  //ECG measure Success  
      StopECG();
      UserDataStruct.End_page=UserEndPage[UserDataStruct.EntryUser];
      ECGStruct.ECGFinal=1;
    }
}
void EcgStructProcess(){
  if(ECGStruct.MeasureTime%128==0){
    if(ECGStruct.MeasureTime%256==0){//每1/2秒SHOW一次...時間與HR交互SHOW
      RTC_Show_Test_Time();
    }else{
      show_HR(ECGStruct.AVGHR);
    }
  }
  
//--------------------------------------------------------減三點平均..可以再減小R波尖點的誤差
/*
  ECGStruct.PointAverage[0]=ECGStruct.PointAverage[1];
  ECGStruct.PointAverage[1]=ECGStruct.PointAverage[2];
  ECGStruct.PointAverage[2]=ECGStruct.ORG_ECGData;
  unsigned int MaxRRAvg=(ECGStruct.PointAverage[0]+ECGStruct.PointAverage[1]+ECGStruct.PointAverage[2])/3;
  unsigned int MaxValue=ECGStruct.ORG_ECGData-MaxRRAvg;
  PutDataNOConvolution(&ORGECG.CBUF,MaxValue);
*/
  PutDataNOConvolution(&ORGECG.CBUF,ECGStruct.ORG_ECGData);
//--------------------------------------------------------    
  //AddValue(&OLC,128-(value>>4)); //TP1:原始數值
  //AddValue(&OLC,128-(var10)); 
  //AddValue(&OLC,(BaseLine>>4));  //TP2:濾波後的數值
  //AddValue(&OLC,128-(B>>4));  //TP2:濾波後的數值
  //AddValue(&OLC,128-(DiffECG.NewValue));  //TP3:微分後的數值
  //AddValue(&OLC,128-(MAVariant.NewValue));  //TP4:標準差的移動平均
  //AddValue(&OLC,128-(Match));  //TP5:MATH FILTER後的數值
  //AddValue(&OLC,128-(DH));   //TP6:DynamicThreshold的數值
//--------------------------------------------------------
  /*  DIGITAL GAIN
  int ShowValue=(LP40ECG.CBUF.NewValue-2048)*ECGStruct.DigitalGain+2048;
  int LP40=Convolution(&BaselineECG.CBUF,LP_40Hz,ShowValue-2048); //經過40Hz低通濾波
  LP40+=2048;
  if(LP40>4095){LP40=4095;}
  if(LP40<0){LP40=0;}
  AddValue(&OLC,128-(LP40>>4));  //TP2:濾波後的數值
  */
  AddValue(&OLC,128-(ECGStruct.ORG_ECGData>>4));  //TP2:濾波後的數值
  WriteByteToFlashBuf(&ECGRawFile,(ECGStruct.ORG_ECGData>>4));   //把原始DATA寫進去
  ECGStruct.MeasureTime++;
  MeasureComplete();
}


void ECG_Acquisition(){
    //int value=AD_Struct.ADCBUF[2];
    int value=GetDataformADQueue(2);
    ECGStruct.ORG_ECGData=value;
/*/--------------------------------------------------FILTER
    int BaseLine=Convolution(&ORGECG.CBUF,HP_Point33Hz,(int)(value)-2048);
    int B=value-BaseLine;
    if(B<0){B=0;}else if(B>4095){B=4095;}
    int LP40=Convolution(&BaselineECG.CBUF,LP_40Hz,B-2048);   */
    //PutDataNOConvolution(&LP40ECG.CBUF,LP40);         //將濾波結果放到LP40ECG
    //int LP40=Convolution(&LP40ECG.CBUF,LP_40Hz,BasLineFilter+2048); //經過40Hz低通濾波
//--------------------------------------------------書玉的ALGORITHM
    int BasLineFilter=ECG_Filter(value); //經過20點平均高通FILTER
//-------------------------------------------------- 
    PutDataNOConvolution(&LP40ECG.CBUF,BasLineFilter);         //將濾波結果放到LP40ECG
    int Diff=Differential(&LP40ECG.CBUF);   //微分並取絕對值
    DiffFilter8BIT(&DiffECG,(Diff>>4));
    unsigned char Match=MatchFilter8BIT(&DiffECG);
    unsigned char DH=DynamicThreshold(Match);
    //AddRRtoComputeSDNN(RRIVSquence.NewValue);

    if (ECGStruct.RDetected==1){
      AddRRIVtoInterFlash(ECGStruct.RRIV);
      ComputeAvgHR(((long)(1.008*(256*60)/ECGStruct.RRIV)),&ECGStruct.RDetected);
      ShortBeepSetTimes(1);  //偵測到R波時
      ComputeDigitalGain();
    }
    if(AD_Struct.ADComplete==1){AD_Struct.ADComplete=0;}
    EcgStructProcess();
//--------------------------------------------------------
}

void StartECG(){
  CreateStaticCoefBuffer(&ORGECG.CBUF,64,ORGBUF);
  //CreateStaticCoefBuffer(&BaselineECG.CBUF,64,BaselineBUF);
  //CreateStaticCoefBuffer(&RRIVSquence,8,RRIVBUF);
  CreateStaticCoefBuffer(&LP40ECG.CBUF,64,LP40BUF);
  CreateStaticCoefBuffer8BIT(&DiffECG,64,DiffBUF);
  CreateStaticCoefBuffer8BIT(&MAVariant,64,MABUF);
  InitECGStruct();
  
  P5OUT |= BIT4;
  ClearScreen();
  CreateChart(0,16,127,47,512,256,&OLC);			//creat picture boundary
  ECGRawFile.AccessPage = Find_Start(UserDataStruct.EntryUser)+32;  //ECG從32PAGE之後開始
  for(int i=0;i<256;i++){
    unsigned long Add=UserDataStruct.Now_Index_page+i;
    if(Add%32==0)BlockErase(Add,0);
  }
  Open_ADC(ECG_Acquisition,2); //ECG是CHANEL 2
}


/*
void SaveADCToBuffer(unsigned char Data){
  if (ECGStruct.NeedSave==0&&ECGStruct.DataIsWritting==0){ //離開FLASHROM 寫入
    if (ECGStruct.TempBufIndex!=0){  //恢復USB存取後，第一次要將資料補滿
      for(unsigned int i=0;i<ECGStruct.TempBufIndex;i++)USB[i]=ECGStruct.TempBUF[i];   //將原本的資料填回去
      ECGStruct.ECG_index=ECGStruct.TempBufIndex;
    }
    ECGStruct.TempBufIndex=0;
    USB[ECGStruct.ECG_index]=Data;  //一般情況
  }else{    //當目前正在使用USB存入FLASHROM時
    ECGStruct.TempBUF[ECGStruct.TempBufIndex]=Data;
    ECGStruct.TempBufIndex++;
  }
}
*/

