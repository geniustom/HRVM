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
    unsigned int aLen=Len-1;  //LEN������2������~�i�o�˰�
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



//�Y�Ʃ�j65536���᭼�[
int InnerProduct(int *Hn,int *Xn,unsigned int Len,unsigned int Front){  //return(16bit int)= Hn[0]*Xn[0]+...+Hn[n]*Xn[n]
    RESHI=0;
    RESLO=0;
    unsigned int aLen=Len-1;  //LEN������2������~�i�o�˰�
    for (int i=0;i<Len;i++){
      MACS=*(Hn++);
      OP2=*(Xn+Front++);
      Front &= aLen;
    }
    return RESHI;  //�u�Ǧ^���줸..�o�˴N���ΦA��65536�^�ӤF
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


















void DiffFilter8BIT(CircultBuffer8BIT *rb,unsigned int ADC_Value){  //�o���ײv�j���o���ŦXECG�S�x������

//----------------------------------------------------------------���зǮt�B�z
  int DAvg=0;
  for (int i=0;i<rb->len;i++){DAvg+=rb->Coef[i];}
  DAvg>>=6; //��64..��������
  unsigned long Variant=0;
  /* �D�w�魼�k�����覡
  for (int i=0;i<rb->len;i++){
    long Law=(rb->Coef[i]-DAvg);
    Variant+=(Law*Law);
  }
  //----------------------------------*/
  // �w�魼�k�����覡..64�I����4���K�I�@MA
  int MA[8]={0};
  for (int i=0;i<8;i++){MA[i]=(rb->Coef[i<<3]-DAvg);}
  Variant=HardwareMACS(MA,MA,8,0);
  //----------------------------------
  Variant>>=3; //��8
  Variant=sqrt(Variant);
  //Variant/=MAVariantThreshold;  //�o�˧���}�ڸ����ۦP�ĪG..���O�B��q�j��
  if (Variant>256){Variant=254;}
  PutDataNOConvolution8BIT(&MAVariant,Variant);  //�N�зǮt�Ȧs��BUFFER��
//----------------------------------------------------------------  
  rb->NewValue=(unsigned char)ADC_Value;
  rb->Coef[rb->Front] = ADC_Value;
  rb->Front++;
  rb->Front &= (rb->len-1);  //modular operation
}

int Differential(CircultBuffer *rb){   //��ECG���G���L��
  unsigned int MOD=rb->len-1;
  long vDiff=
  rb->Coef[(rb->Front-1)&MOD]+      //X(n)
  (rb->Coef[(rb->Front-2)&MOD]<<1)- //2X(n-1)
  rb->Coef[(rb->Front-3)&MOD]-     //X(n-2)
  (rb->Coef[(rb->Front-4)&MOD]<<1); //2X(n-3)
  
  
  //-----------������A��64
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
//-----------------------------------------------�N�зǮt�����ʥ���..�A�N���������o����
  unsigned int MAV=0;
  for(unsigned char i=0;i<MAVariant.len;i++){
    MAV+=MAVariant.Coef[i];
  }
  MAV>>=6;  //��64������
  MAVariant.NewValue=MAV;
  if(MAV>MAVariantThreshold) return 1;   //�������T
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
  ECGStruct.RRIVNextOffset=ECGStruct.MeasureTime+MaxDindex; //�W�@�^�X��
  ECGStruct.RRIV=ECGStruct.RRIVNextOffset-ECGStruct.RRIVOffset;
  ECGStruct.RRIVOffset=ECGStruct.RRIVNextOffset;
}



unsigned char DynamicThreshold(unsigned char MatchValue){
  if ((double)MatchValue>ECGStruct.T0){
    if((ECGStruct.K0-K0init)>((double)ECGRate/((double)PulseHighest/60))){  //PULSE�b�̰����\PULSE���d��
      ECGStruct.T0=MatchValue;
      ECGStruct.K0=K0init;
      ECGStruct.RDetected=1;
      ComputeRRIVOffset();
    }else{
      ECGStruct.K0++;
      ECGStruct.RDetected=0;
      ECGStruct.RRIV++;
    }  //�����O���ŦX�̤p���Z�d��
  }else{  //�S���
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
  //------------�Y�зǮt<10%..�����N�������
  int Vtotal=0;
  for(int i=0;i<RRIVSquence.len;i++){
    int VS=RRIVSquence.Coef[i]-TotalAvg;
    Vtotal+=(VS*VS);
  }
  double Vavg=sqrt(Vtotal/RRIVSquence.len);
  //----------------------------------------
    if (((double)Vavg/0.10)<InputV){return 1;}else{return 0;}
}



unsigned char RWaveDetected(CircultBuffer *rb){  //�ǤJ�n�M�䪺��l�T��..�q��쪺�����I����l�T���M��u�����p�I
  unsigned char RealRR=0;
  if(ECGStruct.RDetected==1){
    int MaxV=0;
    int Front=rb->Front;
    for(int i=0;i<rb->len;i++){   //����l�T���M��u�����p�I
      if (rb->Coef[Front]>=MaxV){
        //MaxIndex=rb->len-i;
        MaxV=rb->Coef[i];
      }
      Front++;
      Front&=(rb->len-1);
    }
    int NewRRIV=ECGStruct.RRIV;
    ECGStruct.RRIV=0; //�k�s
    PutDataOnly(&RRIVSquence,NewRRIV);
    RealRR=ProcessRRIVSquence();
    if(RealRR==1){    //���u����R�i��
      ECGStruct.Pulse=500;
      ECGStruct.HR=floor((double)RRIVSquence.Coef[0]*HRperMinRate);
      //HR++;
      ECGStruct.HR=round(HRperMinRate/(double)RRIVSquence.NewValue);
    }else{ECGStruct.HR=0;}
    //--------------�h�Y�h����..�p���ܲ���..�Y�ܲ���<10%�~�������R�i
    ECGStruct.K0=K0init;//+MaxIndex;
  }
  return RealRR;
}
*/

unsigned char ComputeAvgHR(unsigned char HRvalue,unsigned char *EcgDetect){
    ECGStruct.HRAvgIndex++;
    ECGStruct.HRAvgIndex%=ECG_AVGSize;   //�����B�zHR
    ECGStruct.HRArray[ECGStruct.HRAvgIndex]=HRvalue;
    ECGStruct.RRIV=0; //�k�s
    int Sum=0;
    unsigned char TotalRR=0;
    for(unsigned char i=0;i<ECG_AVGSize;i++){ //�p��RR���j������0������
      if(ECGStruct.HRArray[i]!=0){
        TotalRR++;
        Sum+=ECGStruct.HRArray[i];
      }
    }
    //----------------------�p��зǮt���o�W�L���t��
    Sum=round((double)Sum/TotalRR);
    int MaxDiff=0;
    int DiffRate=5;//floor(Sum*0.05);
    for(unsigned char i=0;i<ECG_AVGSize;i++){
      if(ECGStruct.HRArray[i]!=0){
        MaxDiff=max(abs(ECGStruct.HRArray[i]-Sum),MaxDiff); //��X�̤j���зǮt
      }
    }
    //----------------------�������T���߸�..�s��N��HR���~�t���W�L5..�B�ثeRR�ǦC���ܤ֦��@�b�n�X�k
    if((MaxDiff<DiffRate)&&(TotalRR>=ECG_AVGSizeMin)){
      ECGStruct.AVGHR=Sum; //�N��誺�����Ȧs�J�s��HR
      ECGStruct.NoHRSampleDelay=0;  //�M���NHR SHOW�AOLED�W���ɶ�
      ECGStruct.ShowHRGraph=1; //�A�U�^�X�q�X�R�ߪ���
    }else{   //�����T���߸�..�Y�b768��SAMPLE���LPULSE..�N�NHR�]�^0�öǦ^
      ECGStruct.ShowHRGraph=0; //�A�U�^�X���÷R�ߪ���
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
