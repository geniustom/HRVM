#include "../../DriverExport.h"        // Standard Equations


/**************************************************************************************
                                 DateTime Function
**************************************************************************************/
RTC RTC_Obj;
DateOBJ Date_Obj;

void Init_RTC(){
  Clear_Date();
  Clear_RTC();
}

void Start_RTC(void (*F)()){
  RTC_Obj.F=F;  //call back functon
  Open_TA(TARTC);
}

void Stop_RTC(){
  Close_TA(TARTC);
}

void Clear_Date(){
  Date_Obj.Day=1;
  Date_Obj.Month=9;
  Date_Obj.Year=7;
}

void Clear_RTC(){
  RTC_Obj.mSecond=0;
  RTC_Obj.Second=0;
  RTC_Obj.Minute=0;
  RTC_Obj.Hour=12;
}


void UpdateDate(){ //每呼叫一次，就增加一天，增加的方式符合大小月及閏年
  unsigned char DayofMonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
  if (Date_Obj.Year%4==0)DayofMonth[1]=29; //處理閏年
  Date_Obj.Day++;
  if(Date_Obj.Day>DayofMonth[Date_Obj.Month-1]){
    Date_Obj.Day=1;
    Date_Obj.Month++;
    if (Date_Obj.Month>12){
      Date_Obj.Month=1;
      Date_Obj.Year++;
    }
  }
}


void DisCountTimer(){
  if(RTC_Obj.NowDisCount==0){
    if(RTC_Obj.DisCountTimeOutFP!=0)RTC_Obj.DisCountTimeOutFP();
  }
  RTC_Obj.NowDisCount--;
  RTC_Obj.TestSecond=RTC_Obj.NowDisCount%60;
  RTC_Obj.TestMinute=RTC_Obj.NowDisCount/60;
}

void SetDisCount(unsigned int Sec,void(*TimeOutFP)()){
  RTC_Obj.NowDisCount=Sec;
  RTC_Obj.DisCountTimeOutFP=TimeOutFP;
  DisCountTimer();
}

void UpdateRTCFromRTCChip(){
  I2C_ReadTime();
  DisCountTimer();
}

void UpdateRTC(){  
  RTC_Obj.Second++;
  if (RTC_Obj.Second >= 60){//SECOND
    RTC_Obj.Second = 0;
    RTC_Obj.Minute++;
    if(RTC_Obj.Minute >= 60){//MINUTE
      RTC_Obj.Minute = 0;
      RTC_Obj.Hour++;
      if(RTC_Obj.Hour >= 24){//HOUR
	RTC_Obj.Hour = 0;
        UpdateDate();
      }//HOUR
    }//MINUTE
  }//SECOND
//---------------------------------------TESTRTC
  DisCountTimer();
}

void UpdateMsec(){
  RTC_Obj.mSecond++;
  RTC_Obj.mSecond%=1024;
  if(RTC_Obj.mSecond==0){
    //UpdateRTC();
  }
  if(RTC_Obj.mSecond%128==0){  //每1/5秒就更新一次畫面..以避免時間跑掉
    AddISRMessage(&ISRMSG,ISRPIRORITY_RTC,RTC_Obj.F,ISRMSG_RTC_SEC);
  }
}
