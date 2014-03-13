#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"

//unsigned char USB[517];
UserDataStructure UserDataStruct;
const unsigned long UserStartPage[6]={10240,26624,43008,59392,75776,92160};
const unsigned long UserEndPage[6]={26623,43007,59391,75775,92159,108543};
const unsigned long UserIDPAGEList[6]={2048,2049,2050,2051,2052,2053};


unsigned char Save_UserDataCount(unsigned char UserID){
  unsigned char UserDataCountLIST[6]={0,0,0,0,0,0};
  unsigned char UserDataCount=0;
  Init_Flash();
  for(int i=0;i<6;i++){
    FRead_Data(UserIDPAGEList[i],USB);
    UserDataCount=USB[0];
    if (UserDataCount==0xff){UserDataCount=0;}
    UserDataCountLIST[i]=UserDataCount;
  }
  UserDataCountLIST[UserID]++;  //代表量測完成..多一筆紀錄
  BlockErase(UserIDPAGEList[0],1);
  for(int i=0;i<512;i++)USB[i]=0xff;
  for(int i=0;i<6;i++){   //依序儲存
    USB[0]=UserDataCountLIST[i];
    FSave_Data(UserIDPAGEList[i],USB);
  }
  return UserDataCountLIST[UserID];
};

unsigned long Find_Start(unsigned char UserID){
  unsigned long FAddress;
  unsigned char UserDataCountLIST[6]={0,0,0,0,0,0};
  unsigned char UserDataCount=0;
  Init_Flash();
  for(int i=0;i<6;i++){
    FRead_Data(UserIDPAGEList[i],USB);
    UserDataCount=USB[0];
    if (UserDataCount==0xff){UserDataCount=0;}
    UserDataCountLIST[i]=UserDataCount;
  }
  UserDataStruct.Start_page=UserStartPage[UserID];
  FAddress=UserDataStruct.Start_page+((unsigned long)UserDataCountLIST[UserID]*256);
  UserDataStruct.Now_Index_page=FAddress;
  //UserDataStruct.Now_Index_page=UserDataCountLIST[UserID];
  UserDataStruct.End_page=UserEndPage[UserID];
  return FAddress;
}

void Save_UserIndex(unsigned char UserID){
  Save_UserDataCount(UserID);
  USB[0] = Date_Obj.Year;
  USB[1] = Date_Obj.Month;
  USB[2] = Date_Obj.Day;
  USB[3] = RTC_Obj.Hour;
  USB[4] = RTC_Obj.Minute;
  USB[5] = RTC_Obj.Second;
  FSave_Data(UserDataStruct.Now_Index_page,USB);
}
