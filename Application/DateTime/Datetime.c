#include "../AppExport.h"
#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations


unsigned char RTCSaveCount;
unsigned int DateTimeInFlash[7];  //程式讀取用
const unsigned int DateTimeInFlash0[7]  @RTCAddr0 ={0x00,0x00,0x0C,0x01,0x01,0x07,0xFF}; //改存時間在內部的FLASH
const unsigned int DateTimeInFlash1[7]  @RTCAddr1 ={0x00,0x00,0x0C,0x01,0x01,0x07,0xFF}; //改存時間在內部的FLASH
                                          //秒 , 分 , 時 , 日 , 月 , 年 ,判斷為第一次載入用
//另外..若 DateTimeInFlash[7]=0xAA 代表第一次更新時間..就載入COMPILER時期的時間

void StopDateTime(){
  if(BattStruct.BattDetectState==1){Close_BattDetect();}
}


unsigned int *CompairDateTimeEarly(){  //回傳時間較早的所在指標，讓FLASH寫入
  unsigned char AddrSel=0;
  for(int i=5;i>=0;i--){
    if (DateTimeInFlash0[i]==0xffff){AddrSel=0;break;}
    if (DateTimeInFlash1[i]==0xffff){AddrSel=1;break;}
    if (DateTimeInFlash0[i]>DateTimeInFlash1[i]){AddrSel=0;break;}
    if (DateTimeInFlash1[i]>DateTimeInFlash0[i]){AddrSel=1;break;}
  }
  if(AddrSel==0){return (unsigned int *)RTCAddr1;}
  else{return (unsigned int *)RTCAddr0;}
}

unsigned char CompairDateTimeLast(){  //回傳時間較晚的所在指標，讓FLASH讀取
  unsigned char AddrSel=0,DateTimeOK=1;
  for(int i=5;i>=0;i--){
    if (DateTimeInFlash0[i]==0xffff){AddrSel=1;break;}
    if (DateTimeInFlash1[i]==0xffff){AddrSel=0;break;}
    if (DateTimeInFlash0[i]>DateTimeInFlash1[i]){AddrSel=0;break;}
    if (DateTimeInFlash1[i]>DateTimeInFlash0[i]){AddrSel=1;break;}
  }
  unsigned int *Flash_Ptr;
  if(AddrSel==0){Flash_Ptr=(unsigned int *)RTCAddr0; }
  else{Flash_Ptr=(unsigned int *)RTCAddr1; }

  for(int i=0;i<6;i++){
    DateTimeInFlash[i]=*Flash_Ptr;                       // Write value to flash
    Flash_Ptr++;
    if(DateTimeInFlash[i]==0xff){DateTimeOK=0;break;}
  }
  return DateTimeOK;
}

void SetTime(unsigned char YY1, unsigned char MM1, unsigned char DD1,unsigned char H1, unsigned char M1)
{    
    RTC_Obj.Minute = M1;
    RTC_Obj.Hour = H1;
    Date_Obj.Day = DD1;
    Date_Obj.Month = MM1;
    Date_Obj.Year = YY1;
    SaveTime();
}

void SaveTime(){
  if (AD_Struct.TurnON==1)return;
  unsigned int SaveTimeArray[7]={
    RTC_Obj.Second,
    RTC_Obj.Minute,
    RTC_Obj.Hour,
    Date_Obj.Day,
    Date_Obj.Month,
    Date_Obj.Year,
    0x00
  };
  FCTL2 = FWKEY + FSSEL_2 + FN4 + FN2;      // SMCLK/20+1 = 400K for Flash Timing Generator
//------------------------------------------------------------------------Erease  
  unsigned int *Flash_Ptr;                      // Flash pointer
  Flash_Ptr = (unsigned int *)CompairDateTimeEarly();     // Initialize Flash pointer
  FCTL3 = FWKEY;                        // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                // Set Erase bit
  *Flash_Ptr = 0;                       // Dummy write to erase Flash segment
  FCTL1 = FWKEY;                        // Clear Erase bit
  FCTL3 = FWKEY + LOCK;                 // Reset LOCK bit
//-----------------------------------------------------------------------Write
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  for(int i=0;i<6;i++){
    *Flash_Ptr = SaveTimeArray[i];                       // Write value to flash
    Flash_Ptr++;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit  
}

void SaveTimeToAll(){
  SaveTime();
  SaveTime();
}

/*
{0x00,0x00,0x0C,0x01,0x01,0x07}
  秒0  分1  時2  日3  月4  年5
*/
unsigned long GetDateValue(unsigned char *Date){
  return Date[0]+Date[1]*60+Date[2]*3600+Date[3]*86400+Date[4]*86400*30+Date[5]*86400*365;
//         秒      分         時          日            月               年
}

void NormalizeDateTimeByArray(unsigned char *Date){ //判斷輸入式符合大小月及閏年
  unsigned char NeedNormalize=0;
  if(Date[0]>60)NeedNormalize=1;
  if(Date[1]>60)NeedNormalize=1;
  if(Date[2]>24)NeedNormalize=1;
  if(Date[3]>31)NeedNormalize=1;
  if(Date[4]>12)NeedNormalize=1;
  if(Date[5]>27)NeedNormalize=1;   //2005~2027
  if(Date[5]<7)NeedNormalize=1;
  if(NeedNormalize==1){
    Date[0]=0;
    Date[1]=0;
    Date[2]= 12;
    Date[3]= 1;
    Date[4]= 1;
    Date[5]= 7;
  }
}

void LoadTime(){
  unsigned char RAM[6],IFLASH1[6],IFLASH2[6],EFLASH[6];
  unsigned char *RESULT;
  unsigned long AA,BB,CC,DD,EE=0;
//先檢查記憶體,2塊內部FLASHROM,讀外部ROM
//----------------------------------------------記憶體-----------------------
  RAM[0]=RTC_Obj.Second;
  RAM[1]=RTC_Obj.Minute;
  RAM[2]=RTC_Obj.Hour;
  RAM[3]=Date_Obj.Day;
  RAM[4]=Date_Obj.Month;
  RAM[5]=Date_Obj.Year;  
//----------------------------------------------內部FLASHROM1-----------------------
  for(unsigned char i=0;i<6;i++)IFLASH1[i]=DateTimeInFlash0[i];
//----------------------------------------------內部FLASHROM2-----------------------
  for(unsigned char i=0;i<6;i++)IFLASH2[i]=DateTimeInFlash1[i];
//----------------------------------------------外部ROM-----------------------
/*/----------------------------------
  USB[0]=5;
  USB[1]=6;
  USB[2]=7;
  USB[3]=8;
  USB[4]=9;
  Block_Erase(Page_RTC);
  save_data(Page_RTC,USB);
  USB[0]=0;
  USB[1]=0;
  USB[2]=0;
  USB[3]=0;
  USB[4]=0;
//----------------------------------*/
  FRead_Data(Page_RTC,USB);
  EFLASH[0]= USB[5];
  EFLASH[1]= USB[4];
  EFLASH[2]= USB[3];
  EFLASH[3]= USB[2];
  EFLASH[4]= USB[1];
  EFLASH[5]= USB[0];
//---------------------------------------------正規化成標準格式---------------------
  NormalizeDateTimeByArray(RAM);
  NormalizeDateTimeByArray(IFLASH1);
  NormalizeDateTimeByArray(IFLASH2);
  NormalizeDateTimeByArray(EFLASH);
//-----------------------------------------取得時間絕對秒數----------------------------------
  AA=GetDateValue(RAM);
  BB=GetDateValue(IFLASH1);
  CC=GetDateValue(IFLASH2);
  DD=GetDateValue(EFLASH);
  EE=max(EE,AA);EE=max(EE,BB);EE=max(EE,CC);EE=max(EE,DD);
  //EE=DD; //只讀外部FLASH
  if(EE==AA){RESULT=(unsigned char *)RAM;}
  if(EE==BB){RESULT=(unsigned char *)IFLASH1;}
  if(EE==CC){RESULT=(unsigned char *)IFLASH2;}
  if(EE==DD){RESULT=(unsigned char *)EFLASH;}
//---------------------------------------------------------------------------
  RTC_Obj.Second = RESULT[0];
  RTC_Obj.Minute = RESULT[1];
  RTC_Obj.Hour = RESULT[2];
  Date_Obj.Day = RESULT[3];
  Date_Obj.Month = RESULT[4];
  Date_Obj.Year = RESULT[5];
}

void ShowTimeInFlash(){
  FRead_Data(Page_RTC,USB);
  RTC_Obj.Second=USB[5];
  RTC_Obj.Minute=USB[4];
  RTC_Obj.Hour=USB[3];
  Date_Obj.Day=USB[2];
  Date_Obj.Month=USB[1];
  Date_Obj.Year=USB[0];  
}

