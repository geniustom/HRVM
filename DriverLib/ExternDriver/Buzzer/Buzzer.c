#include "../../DriverExport.h"

unsigned char BeepID,BeepID2;
unsigned int BeepCount=0;
unsigned int BeepCount2=0;
unsigned char BeepTimes=0;


void BuzzerOn(){
  P6DIR|=BuzzerPin;
  P6OUT&=~BuzzerPin;  //�ٹq
  Open_TA(TABuzzer);
}

void BuzzerOff(){
  Close_TA(TABuzzer);
  P6DIR&=~BuzzerPin;
  P6OUT&=~BuzzerPin;  //�ٹq
}


void Beep1Sec(){
  
  if(BeepID!=0){
    ReleaseTimer(BeepID);  //�j������..�_�h�쥻���Y�٦b�T..�N�|����1��TIMER����
    BeepID=0;
    BuzzerOff();  //�����ìٹq
  }
  
  BuzzerOn();
  BeepID=TakeTimer(RANDOM,3000,SINGLE,BuzzerOff);
}


void ShortBeep(){
  BeepCount=0;
  
  if(BeepID!=0){
    ReleaseTimer(BeepID);  //�j������..�_�h�쥻���Y�٦b�T..�N�|����1��TIMER����
    BeepID=0;
    BuzzerOff();  //�����ìٹq
  }
  if (BeepCount2>=BeepTimes){
    ReleaseTimer(BeepID2);
    BeepID2=0;
    BuzzerOff();  //�����ìٹq
  }else{
    BuzzerOn();
    BeepID=TakeTimer(RANDOM,1500,SINGLE,BuzzerOff);
  }
  BeepCount2++; 
}

void ShortBeepSetTimes(unsigned char Times){
  BeepTimes=Times;
  if(BeepID2!=0)ReleaseTimer(BeepID2); //�j������
  if(BeepID!=0)ReleaseTimer(BeepID);  //�j������
  BuzzerOff();
  BeepID2=0;
  BeepID=0;
  BeepCount2=0;
  BeepCount=0;
  BeepID2=TakeTimer(RANDOM,3000,0,ShortBeep);  //9830=32768/3
}

