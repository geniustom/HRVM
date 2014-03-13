#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"

unsigned char RING_BUFF[517];
UserDataStructure UserDataStruct;
//const unsigned int UserStartPage[6]={2048,10240,18432,26624,34816,43008};
//const unsigned int UserEndPage[6]={10208,18400,26592,34784,42976,51168};
const unsigned int UserStartPage[6]={2048,10240,18432,26624,34816,43008};
const unsigned int UserEndPage[6]={10239,18431,26623,34815,43007,51199};
const unsigned char UserIDPAGEList[6]={32,33,34,35,36,37};



void Save_UserDataCount(unsigned char UserID){
  unsigned char UserDataCountLIST[6]={0,0,0,0,0,0};
  unsigned char UserDataCount=0;
  for(int i=0;i<6;i++){
    FRead_Data(UserIDPAGEList[i],USB);
    UserDataCount=USB[0];
    if (UserDataCount==0xff){UserDataCount=0;}
    UserDataCountLIST[i]=UserDataCount;
  }
  UserDataCountLIST[UserID]++;  //�N��q������..�h�@������
  BlockErase(UserIDPAGEList[0],1);
  for(int i=0;i<6;i++){   //�̧��x�s
    USB[0]=UserDataCountLIST[i];
    FSave_Data(UserIDPAGEList[i],USB);
  }
};

unsigned char Get_UserDataCount(){
  //�N�O����̭�����Ū�X��
  unsigned char TotalDataCount=0;
  for(int i=0;i<6;i++){
    FRead_Data(UserIDPAGEList[i],USB);
    unsigned char UserDataCount=USB[0];
    if (UserDataCount==0xff){UserDataCount=0;}
    TotalDataCount+=UserDataCount;
  }
  return TotalDataCount;
};


void Clear_UserDataCount(unsigned char UserID){
  unsigned char UserDataCountLIST[6]={0,0,0,0,0,0};
  unsigned char UserDataCount=0;
  for(int i=0;i<6;i++){
    FRead_Data(UserIDPAGEList[i],USB);
    UserDataCount=USB[0];
    if (UserDataCount==0xff){UserDataCount=0;}
    UserDataCountLIST[i]=UserDataCount;
  }
  UserDataCountLIST[UserID]=0;  //�R���ӨϥΪ̰O�����ƶq
  BlockErase(UserIDPAGEList[0],1);
  for(int i=0;i<6;i++){   //�̧��x�s
    USB[0]=UserDataCountLIST[i];
    FSave_Data(UserIDPAGEList[i],USB);
  }
};



int Find_Start(char User){
  unsigned int Page_offset;
  unsigned int i;
  UserDataStruct.Start_page=UserStartPage[User];
  UserDataStruct.End_page=UserEndPage[User];
  
  FRead_Data(UserDataStruct.End_page,RING_BUFF);
 // FRead_Data(Start_page,RING_BUFF);
  
  for (i=0;i<512;i+=16){
    if ((RING_BUFF[i+21]==0xFF && RING_BUFF[i+22]==0xFF) || (RING_BUFF[i+21]==0x00 && RING_BUFF[i+22]==0x00))
    {
      // �]��NAND Flash�̭��S�g��ơA�t�~�W�h���w�g�⥩�X�����αư��C
    }else{
      if (RING_BUFF[i+7]==RING_BUFF[i+21] && RING_BUFF[i+8]==RING_BUFF[i+22]){
        Page_offset = RING_BUFF[i+7] * 256 + RING_BUFF[i+8];    
        UserDataStruct.Index_page = (i / 16) + 1;
      }
    }
  }
  
  //�p�G�s���F�A�N�M�ŰO����ñq�Y�}�l�s
  if (Page_offset > 1536 || UserDataStruct.Index_page > 20){
    BlockErase(UserDataStruct.End_page,1);
    Page_offset = 0;
    UserDataStruct.Index_page = 0;
  }
  return Page_offset;
}

//�N�ϥΪ̯��ަs��NAND Flash�̭�
void Save_UserIndex(int Temp_num)
{
  //�N�O����̭�����Ū�X��
  //FRead_Data(Start_page,RING_BUFF);
  FRead_Data(UserDataStruct.End_page,RING_BUFF); 
  //Data & Time
  if (UserDataStruct.Index_page!=0)
  {
    unsigned long P=UserDataStruct.Index_page<<4;
    RING_BUFF[P] = Date_Obj.Year;
    RING_BUFF[P+1] = Date_Obj.Month;
    RING_BUFF[P+2] = Date_Obj.Day;
    RING_BUFF[P+3] = RTC_Obj.Hour;
    RING_BUFF[P+4] = RTC_Obj.Minute;   
    //Page Start
    RING_BUFF[P+5] = RING_BUFF[P-9];
    RING_BUFF[P+6] = RING_BUFF[P-8];
    //Page Stop
    UserDataStruct.Page += RING_BUFF[P-8];
    UserDataStruct.Page += RING_BUFF[P-9] * 256;
    RING_BUFF[P+7] = UserDataStruct.Page >> 8;
    RING_BUFF[P+8] = UserDataStruct.Page % 256;  
    //Temp Value Save
    RING_BUFF[P+9] = Temp_num / 100;
    RING_BUFF[P+10] = Temp_num %  100; 
    //Next Page Start
    RING_BUFF[P+21] = UserDataStruct.Page >> 8;
    RING_BUFF[P+22] = UserDataStruct.Page % 256;  
  }
  else
  {
    unsigned long P=UserDataStruct.Index_page;
    RING_BUFF[P]   =  Date_Obj.Year;
    RING_BUFF[P+1] =  Date_Obj.Month;
    RING_BUFF[P+2] =  Date_Obj.Day;
    RING_BUFF[P+3] =  RTC_Obj.Hour;
    RING_BUFF[P+4] =  RTC_Obj.Minute;   
    //Page Start
    RING_BUFF[P+5] = 0x00;
    RING_BUFF[P+6] = 0x00;
    //Page Stop
    RING_BUFF[P+7] = UserDataStruct.Page >> 8;
    RING_BUFF[P+8] = UserDataStruct.Page % 256;   
    //Temp Value Save
    RING_BUFF[(UserDataStruct.Index_page<<4)+9] = Temp_num / 100;
    RING_BUFF[(UserDataStruct.Index_page<<4)+10] = Temp_num %  100;     
    //Next Page Start
    RING_BUFF[P+21] = UserDataStruct.Page >> 8;
    RING_BUFF[P+22] = UserDataStruct.Page % 256;       
  }
  //�M�ŰO����A�H�K�i��g�J�ʧ@
  //BlockErase(Start_page);
  //�N�ȼg�^�O���餺
  BlockErase(UserDataStruct.End_page,1);
  FSave_Data(UserDataStruct.End_page,RING_BUFF);
}


