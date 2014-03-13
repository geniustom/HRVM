#include <math.h>
#include <stdlib.h>
#include "../AppExport.h"


CircultBuffer8BIT MAVariant;
unsigned char MABUF[64];
ECGStructure ORGECG;
int ORGBUF[64];

//CircultBuffer RRIVSquence;
//int RRIVBUF[64];



long round(double x){
  if(x-(x/1)>=0.5){return floor(x+1);}
  else{return floor(x);}
}

void CreateCoefBuffer(CircultBuffer *rb,unsigned int Len){
  //rb = (CircultBuffer *)malloc(sizeof(CircultBuffer));
  if (rb != 0){
     rb->Front=0;
     rb->len=Len;
     rb->Coef = (int *)malloc(sizeof(unsigned int)*Len);
     /*
     if (rb != 0){
       for(int i=0;i<Len;i++){rb->Coef[i]=2048;}
     }
     */
  }
}

void CreateStaticCoefBuffer(CircultBuffer *rb,unsigned int Len,int *Databuff){
  //rb = (CircultBuffer *)malloc(sizeof(CircultBuffer));
  if (rb != 0){
     rb->Front=0;
     rb->len=Len;
     rb->Coef = Databuff;
     /*
     if (rb != 0){
       for(int i=0;i<Len;i++){rb->Coef[i]=2048;}
     }
     */
  }
}

void CreateStaticCoefBuffer8BIT(CircultBuffer8BIT *rb,unsigned int Len,unsigned char *Databuff){
  //rb = (CircultBuffer *)malloc(sizeof(CircultBuffer));
  if (rb != 0){
     rb->Front=0;
     rb->len=Len;
     rb->Coef = Databuff;
     /*
     if (rb != 0){
       for(int i=0;i<Len;i++){rb->Coef[i]=128;}
     }
     */
  }
}

void CreateCoefBuffer8BIT(CircultBuffer8BIT *rb,unsigned int Len){
  //rb = (CircultBuffer *)malloc(sizeof(CircultBuffer));
  if (rb != 0){
     rb->Front=0;
     rb->len=Len;
     rb->Coef = (unsigned char *)malloc(sizeof(unsigned char)*Len);
     /*
     if (rb != 0){
       for(int i=0;i<Len;i++){rb->Coef[i]=128;}
     }
     */
  }
}


long HardwareMACS(int *Hn,int *Xn,unsigned int Len,unsigned int Front){  //return(16bit int)= Hn[0]*Xn[0]+...+Hn[n]*Xn[n]
    RESHI=0;
    RESLO=0;
    unsigned int aLen=Len-1;  //LEN必須為2的次方才可這樣做
    for (int i=0;i<Len;i++){
      MACS=*(Hn++);
      OP2=*(Xn+Front++);
      Front &= aLen;
    }
    long ReturnValue= (long)RESHI;
    ReturnValue<<=16;
    ReturnValue|=(long)RESLO;
    return ReturnValue;
}



//係數放大65536倍後乘加
int InnerProduct(int *Hn,int *Xn,unsigned int Len,unsigned int Front){  //return(16bit int)= Hn[0]*Xn[0]+...+Hn[n]*Xn[n]
    RESHI=0;
    RESLO=0;
    unsigned int aLen=Len-1;  //LEN必須為2的次方才可這樣做
    for (int i=0;i<Len;i++){
      MACS=*(Hn++);
      OP2=*(Xn+Front++);
      Front &= aLen;
    }
    return RESHI;  //只傳回高位元..這樣就不用再除65536回來了
}
/*
        |hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh| <-h(0)~h(n)

        |++++++++++++++++++++++++x------------------| <-x(0)~x(n)
                                 ^^
                                F  F+1   (F=New ADCvalue , F+1=oldest value)
*/

long Convolution(CircultBuffer *rb,int const *Hn,int ADC_Value){ //Input ADCoutput,Output FIR coef
  long Sum=0;
  rb->NewValue=ADC_Value;
  rb->Coef[rb->Front] = ADC_Value;
  rb->Front++;
  rb->Front &=(rb->len-1);  //modular operation
  Sum=InnerProduct((int*)Hn,rb->Coef,rb->len,rb->Front);
  return Sum;
}


void PutDataNOConvolution(CircultBuffer *rb,int ADC_Value){
  ADC_Value+=2048;
  if(ADC_Value>4095)ADC_Value=4095;
  if(ADC_Value<-4095)ADC_Value=-4095;
  rb->NewValue=ADC_Value;
  rb->Coef[rb->Front] = ADC_Value;
  rb->Front++;
  rb->Front &=(rb->len-1);  //modular operation
}

void PutDataOnly(CircultBuffer *rb,int ADC_Value){
  rb->NewValue=ADC_Value;
  rb->Coef[rb->Front] = ADC_Value;
  rb->Front++;
  rb->Front &=(rb->len-1);  //modular operation
}

void PutDataNOConvolution8BIT(CircultBuffer8BIT *rb,unsigned char ADC_Value){
  rb->NewValue=ADC_Value;
  rb->Coef[rb->Front] = ADC_Value;
  rb->Front++;
  rb->Front &=(rb->len-1);  //modular operation
}



void ReleaseCoefBuffer(CircultBuffer *rb){
  if (rb != 0){
    free(rb->Coef);
    rb->Coef = 0;
    rb = 0;
  }
}

void ReleaseCoefBuffer8BIT(CircultBuffer8BIT *rb){
  if (rb != 0){
    free(rb->Coef);
    rb->Coef = 0;
    rb = 0;
  }
}


















void DiffFilter8BIT(CircultBuffer8BIT *rb,unsigned int ADC_Value){  //濾掉斜率大但卻不符合ECG特徵的部分

//----------------------------------------------------------------做標準差處理
  int DAvg=0;
  for (int i=0;i<rb->len;i++){DAvg+=rb->Coef[i];}
  DAvg>>=6; //除64..取平均值
  unsigned long Variant=0;
  /* 非硬體乘法器的方式
  for (int i=0;i<rb->len;i++){
    long Law=(rb->Coef[i]-DAvg);
    Variant+=(Law*Law);
  }
  //----------------------------------*/
  // 硬體乘法器的方式..64點間格4取八點作MA
  int MA[8]={0};
  for (int i=0;i<8;i++){MA[i]=(rb->Coef[i<<3]-DAvg);}
  Variant=HardwareMACS(MA,MA,8,0);
  //----------------------------------
  Variant>>=3; //除8
  Variant=sqrt(Variant);
  //Variant/=MAVariantThreshold;  //這樣坐跟開根號有相同效果..但是運算量大減
  if (Variant>256){Variant=254;}
  PutDataNOConvolution8BIT(&MAVariant,Variant);  //將標準差值存到BUFFER中
//----------------------------------------------------------------  
  rb->NewValue=(unsigned char)ADC_Value;
  rb->Coef[rb->Front] = ADC_Value;
  rb->Front++;
  rb->Front &= (rb->len-1);  //modular operation
}

int Differential(CircultBuffer *rb){   //對ECG做二階微分
  unsigned int MOD=rb->len-1;
  long vDiff=
  rb->Coef[(rb->Front-1)&MOD]+      //X(n)
  (rb->Coef[(rb->Front-2)&MOD]<<1)- //2X(n-1)
  rb->Coef[(rb->Front-3)&MOD]-     //X(n-2)
  (rb->Coef[(rb->Front-4)&MOD]<<1); //2X(n-3)
  
  
  //-----------取平方再除64
  //vDiff>>=1;
  //vDiff=vDiff*vDiff;
  //vDiff>>=4;
  vDiff=abs(vDiff);
  if(vDiff>4095){vDiff=4095;}
  return vDiff;
}


unsigned char MatchFilter8BIT(CircultBuffer8BIT *rb){
  unsigned char MaxLeft=0,MaxRight=0,MaxMid=0;
  unsigned char MValue;
  int RESULT;
  unsigned char Front=rb->Front;
//-----------------------------------------------將標準差做移動平均..再將高的部分濾除掉
  unsigned int MAV=0;
  for(unsigned char i=0;i<MAVariant.len;i++){
    MAV+=MAVariant.Coef[i];
  }
  MAV>>=6;  //除64做平均
  MAVariant.NewValue=MAV;
  if(MAV>MAVariantThreshold) return 1;   //視為雜訊
//-----------------------------------------------  
  for(unsigned char i=0;i<rb->len;i++){
    MValue=rb->Coef[(Front+i)&(rb->len-1)]; 
    if(i<17){MaxLeft=max(MaxLeft,MValue);}
    else if(i>47){MaxRight=max(MaxRight,MValue);}
    else{MaxMid=max(MaxMid,MValue);}   //19~44
  }
  RESULT=(MaxMid-MaxLeft)+(MaxMid-MaxRight);
  
  if(RESULT<=0){RESULT=0;}
  else if (RESULT>=255){RESULT=255;}
  return ((unsigned char)RESULT);
}


void ComputeRRIVOffset(){
  int MaxData=0;
  int MaxDindex=0;
  for(int i=0;i<64;i++){
    if(ORGBUF[i]>MaxData){
      MaxDindex=i;
      MaxData=ORGBUF[i];
    }
  }
  MaxDindex=MaxDindex-ORGECG.CBUF.Front;
  if (MaxDindex<0)MaxDindex+=64;
  ECGStruct.RRIVNextOffset=ECGStruct.MeasureTime+MaxDindex; //上一回合的
  ECGStruct.RRIV=ECGStruct.RRIVNextOffset-ECGStruct.RRIVOffset;
  ECGStruct.RRIVOffset=ECGStruct.RRIVNextOffset;
}



unsigned char DynamicThreshold(unsigned char MatchValue){
  if ((double)MatchValue>ECGStruct.T0){
    if((ECGStruct.K0-K0init)>((double)ECGRate/((double)PulseHighest/60))){  //PULSE在最高允許PULSE之範圍內
      ECGStruct.T0=MatchValue;
      ECGStruct.K0=K0init;
      ECGStruct.RDetected=1;
      ComputeRRIVOffset();
    }else{
      ECGStruct.K0++;
      ECGStruct.RDetected=0;
      ECGStruct.RRIV++;
    }  //抓到但是不符合最小間距範圍
  }else{  //沒抓到
    ECGStruct.K0++;
    ECGStruct.T0=ECGStruct.T0-(ECGStruct.T0/ECGStruct.K0);
    ECGStruct.RDetected=0;
    ECGStruct.RRIV++;
    if(ECGStruct.T0<MinDynamicThreshold){ECGStruct.T0=MinDynamicThreshold;}
  }
  return (unsigned char)ECGStruct.T0;
}




/*

unsigned char ProcessRRIVSquence(){
  unsigned int MaxRR=0,MinRR=65535,TotalAvg=0;
  for(int i=0;i<RRIVSquence.len;i++){ //find max,min and compute total
    MaxRR=max(MaxRR,RRIVSquence.Coef[i]);
    MinRR=min(MinRR,RRIVSquence.Coef[i]);
    TotalAvg+=RRIVSquence.Coef[i];
  }
  TotalAvg=(TotalAvg-MaxRR-MinRR)/(RRIVSquence.len-2);
  int InputV=RRIVSquence.NewValue;
  RRIVSquence.NewValue=TotalAvg;
  //------------若標準差<10%..本次就視為抓到
  int Vtotal=0;
  for(int i=0;i<RRIVSquence.len;i++){
    int VS=RRIVSquence.Coef[i]-TotalAvg;
    Vtotal+=(VS*VS);
  }
  double Vavg=sqrt(Vtotal/RRIVSquence.len);
  //----------------------------------------
    if (((double)Vavg/0.10)<InputV){return 1;}else{return 0;}
}



unsigned char RWaveDetected(CircultBuffer *rb){  //傳入要尋找的原始訊號..從抓到的那個點往原始訊號尋找真正的峰點
  unsigned char RealRR=0;
  if(ECGStruct.RDetected==1){
    int MaxV=0;
    int Front=rb->Front;
    for(int i=0;i<rb->len;i++){   //往原始訊號尋找真正的峰點
      if (rb->Coef[Front]>=MaxV){
        //MaxIndex=rb->len-i;
        MaxV=rb->Coef[i];
      }
      Front++;
      Front&=(rb->len-1);
    }
    int NewRRIV=ECGStruct.RRIV;
    ECGStruct.RRIV=0; //歸零
    PutDataOnly(&RRIVSquence,NewRRIV);
    RealRR=ProcessRRIVSquence();
    if(RealRR==1){    //抓到真正的R波時
      ECGStruct.Pulse=500;
      ECGStruct.HR=floor((double)RRIVSquence.Coef[0]*HRperMinRate);
      //HR++;
      ECGStruct.HR=round(HRperMinRate/(double)RRIVSquence.NewValue);
    }else{ECGStruct.HR=0;}
    //--------------去頭去尾後..計算變異度..若變異度<10%才視為抓到R波
    ECGStruct.K0=K0init;//+MaxIndex;
  }
  return RealRR;
}
*/

unsigned char ComputeAvgHR(unsigned char HRvalue,unsigned char *EcgDetect){
    ECGStruct.HRAvgIndex++;
    ECGStruct.HRAvgIndex%=ECG_AVGSize;   //環狀處理HR
    ECGStruct.HRArray[ECGStruct.HRAvgIndex]=HRvalue;
    ECGStruct.RRIV=0; //歸零
    int Sum=0;
    unsigned char TotalRR=0;
    for(unsigned char i=0;i<ECG_AVGSize;i++){ //計算RR間隔不等於0的部份
      if(ECGStruct.HRArray[i]!=0){
        TotalRR++;
        Sum+=ECGStruct.HRArray[i];
      }
    }
    //----------------------計算標準差不得超過正負五
    Sum=round((double)Sum/TotalRR);
    int MaxDiff=0;
    int DiffRate=5;//floor(Sum*0.05);
    for(unsigned char i=0;i<ECG_AVGSize;i++){
      if(ECGStruct.HRArray[i]!=0){
        MaxDiff=max(abs(ECGStruct.HRArray[i]-Sum),MaxDiff); //算出最大的標準差
      }
    }
    //----------------------視為正確的心跳..連續N個HR都誤差不超過5..且目前RR序列中至少有一半要合法
    if((MaxDiff<DiffRate)&&(TotalRR>=ECG_AVGSizeMin)){
      ECGStruct.AVGHR=Sum; //將剛剛的平均值存入新的HR
      ECGStruct.NoHRSampleDelay=0;  //清除將HR SHOW再OLED上的時間
      ECGStruct.ShowHRGraph=1; //再下回合秀出愛心的圖
    }else{   //不正確的心跳..若在768個SAMPLE內無PULSE..就將HR設回0並傳回
      ECGStruct.ShowHRGraph=0; //再下回合隱藏愛心的圖
      /*if(ECGStruct.NoHRSampleDelay>=SampleDealyBound){
        ECGStruct.NoHRSampleDelay=0;
        ECGStruct.AVGHR=0;
      }
      */
    }
    ECGStruct.NoHRDelay=0;
    if(ECGStruct.AVGHR==0)*EcgDetect=0;
    return ECGStruct.AVGHR;
}

/*
void Filter_Pulse(int *value){
    // Filter Pulse
    ECGStruct.Filter_Pulse[2]=ECGStruct.Filter_Pulse[1];
    ECGStruct.Filter_Pulse[1]=ECGStruct.Filter_Pulse[0];
    ECGStruct.Filter_Pulse[0]=*value;
    
    if (fabs(ECGStruct.Filter_Pulse[2]-ECGStruct.Filter_Pulse[1])>Filter_Pulse_ref && 
       fabs(ECGStruct.Filter_Pulse[1]-ECGStruct.Filter_Pulse[0])>Filter_Pulse_ref){   
          ECGStruct.Filter_Pulse[1] = (ECGStruct.Filter_Pulse[2] + ECGStruct.Filter_Pulse[0]) << 1;
    }
    *value=ECGStruct.Filter_Pulse[1];    
}
*/

int ECG_Filter(int value){
    // Smooth function test
    long var10=0;
    for(int i=Smooth_Val-1;i>0;i--){
      var10 += ECGStruct.Smooth[i];
      ECGStruct.Smooth[i] = ECGStruct.Smooth[i-1];
    }
    ECGStruct.Smooth[0] = value;
    var10 += value; 
    var10 /= Smooth_Val; 
    ECGStruct.BaseLine=var10;
    var10 = (value - ECGStruct.BaseLine);
     

    return (int)var10;
}
