#include "../../DriverExport.h"

unsigned char BeepID,BeepID2;
unsigned int BeepCount=0;
unsigned int BeepCount2=0;
unsigned char BeepTimes=0;


void BuzzerOn(){
  P6DIR|=BuzzerPin;
  P6OUT&=~BuzzerPin;  //省電
  Open_TA(TABuzzer);
}

void BuzzerOff(){
  Close_TA(TABuzzer);
  P6DIR&=~BuzzerPin;
  P6OUT&=~BuzzerPin;  //省電
}


void Beep1Sec(){
  
  if(BeepID!=0){
    ReleaseTimer(BeepID);  //強迫停止..否則原本的若還在響..將會佔用1個TIMER不放
    BeepID=0;
    BuzzerOff();  //關閉並省電
  }
  
  BuzzerOn();
  BeepID=TakeTimer(RANDOM,3000,SINGLE,BuzzerOff);
}


void ShortBeep(){
  BeepCount=0;
  
  if(BeepID!=0){
    ReleaseTimer(BeepID);  //強迫停止..否則原本的若還在響..將會佔用1個TIMER不放
    BeepID=0;
    BuzzerOff();  //關閉並省電
  }
  if (BeepCount2>=BeepTimes){
    ReleaseTimer(BeepID2);
    BeepID2=0;
    BuzzerOff();  //關閉並省電
  }else{
    BuzzerOn();
    BeepID=TakeTimer(RANDOM,1500,SINGLE,BuzzerOff);
  }
  BeepCount2++; 
}

void ShortBeepSetTimes(unsigned char Times){
  BeepTimes=Times;
  if(BeepID2!=0)ReleaseTimer(BeepID2); //強迫停止
  if(BeepID!=0)ReleaseTimer(BeepID);  //強迫停止
  BuzzerOff();
  BeepID2=0;
  BeepID=0;
  BeepCount2=0;
  BeepCount=0;
  BeepID2=TakeTimer(RANDOM,3000,0,ShortBeep);  //9830=32768/3
}

