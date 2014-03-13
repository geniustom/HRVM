#include "MenuUI.h"
#include "MAP.h"
#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"

const MenuNode Standby;
const MenuNode Main_DateTime,Main_ECG,Main_Thermal;         //主選單3
const MenuNode ECG_Main,ECG_Success,ECG_Abort;              //ECG狀態
const MenuNode Main_User,User1,User2,User3,User4,User5,User6; //User Select

                                   //目前所處的選單
void (*WhenExitMenuNode)(void); //進入節點的FUNTION POINT


const MenuNode Standby={(unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)&Main_DateTime,//&PowerDown
                         &PowerDown,0,"STANDBY",CleenScreen,DynamicScreen};

const MenuNode Main_DateTime={(unsigned int*)&Main_ECG,(unsigned int*)&Main_User,(unsigned int*)&Standby,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)&Standby,
                         0/*&MainDateTime*/,&StopDateTime,"MAIN DATETIME",(unsigned int)&OLED_SHOW_EMPTY,DynamicScreen};  

const MenuNode Main_ECG={(unsigned int*)&Main_User,(unsigned int*)&Main_DateTime,(unsigned int*)&ECG_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"MAIN ECG",(unsigned int)&OLED_SHOW_Test_Heart,StaticScreen};  

const MenuNode ECG_Main={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&ECG_Abort,
                       (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                        StartECG,StopECG,"ECG MAIN",CleenScreen,DynamicScreen|NoBuzzer};  

const MenuNode ECG_Success={(unsigned int*)&ECG_Main,(unsigned int*)&Main_DateTime,(unsigned int*)0,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"ECG SUCCESS",(unsigned int)&OLED_SHOW_Success,StaticScreen};    

const MenuNode ECG_Abort={(unsigned int*)&ECG_Main,(unsigned int*)&Main_DateTime,(unsigned int*)0,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"ECG ABORT",(unsigned int)&OLED_SHOW_Fail,StaticScreen};    


const MenuNode Main_User={(unsigned int*)&Main_DateTime,(unsigned int*)&Main_ECG,(unsigned int*)&User1,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"NOW USER",(unsigned int)&OLED_SHOW_User,StaticScreen};  

const MenuNode User1={(unsigned int*)&User6,(unsigned int*)&User2,(unsigned int*)&Main_User,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &User_Entry,0,"USER 1",(unsigned int)&OLED_SHOW_User1,StaticScreen};

const MenuNode User2={(unsigned int*)&User1,(unsigned int*)&User3,(unsigned int*)&Main_User,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &User_Entry,0,"USER 2",(unsigned int)&OLED_SHOW_User2,StaticScreen};

const MenuNode User3={(unsigned int*)&User2,(unsigned int*)&User4,(unsigned int*)&Main_User,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &User_Entry,0,"USER 3",(unsigned int)&OLED_SHOW_User3,StaticScreen};

const MenuNode User4={(unsigned int*)&User3,(unsigned int*)&User5,(unsigned int*)&Main_User,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &User_Entry,0,"USER 4",(unsigned int)&OLED_SHOW_User4,StaticScreen};

const MenuNode User5={(unsigned int*)&User4,(unsigned int*)&User6,(unsigned int*)&Main_User,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &User_Entry,0,"USER 5",(unsigned int)&OLED_SHOW_User5,StaticScreen};  

const MenuNode User6={(unsigned int*)&User5,(unsigned int*)&User1,(unsigned int*)&Main_User,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &User_Entry,0,"USER 6",(unsigned int)&OLED_SHOW_User6,StaticScreen};  
  






MenuNode CurrentNode;
unsigned char PowerOffAfter5Sec=0;


void Draw_RDMode(){
  ClearScreen();
  ClearBlock(0,0,128,16);
  Print(VerString,13,2,2,8);
  if((CurrentNode.OtherSetting&DontPrintName)==0)Print(CurrentNode.NAME,13,2,30,8);
}

void User_Entry()
{
  if(CurrentNode.Page!= 0x27)
  {
    UserDataStruct.EntryUser = CurrentNode.NAME[5]-0x31;  //直接把ASCII的值取出來..轉換後再減一
  }
}

void Draw_BG_Image(){
   unsigned char K[11]={'0','1','2','3','4','5','6','7','8','9',':'};   
   // RD MODE SELECT
   if((CurrentNode.OtherSetting&RDModeNode)!=0){
    Draw_RDMode();
    return;
   }
//---------------------------------------------------------------------------------USER MODE 中的例外狀況
   if(CurrentNode.NAME=="ECG MAIN")return;
   if (CurrentNode.Page == CleenScreen)  ClearScreen(); //CLEAR
   if((CurrentNode.OtherSetting&LCDTEST)==LCDTEST) FullScreen();
   else Draw_Screen((unsigned char *)CurrentNode.Page);  // Show BackGround Image ; 
           
   if (CurrentNode.NAME!="MAIN DATETIME"&&(CurrentNode.OtherSetting&LCDTEST)!=LCDTEST){
      ClearBlock(0,0,128,16);
      Print(CurrentNode.NAME,13,2,2,8); //只有時間不顯示標題
   }

  if(CurrentNode.NAME=="NOW USER") Print(&K[UserDataStruct.EntryUser+1],1,80,2,8);  //在選擇USER的地方顯示目前是哪個USER
}


void CheckButtonRelease(MenuNode *MenuChange){
  if(CurrentNode.NAME==Standby.NAME&&MenuChange==0x00){
    //表示目前在STANDBY模式下按了無效的按鈕..即RESET
    Reset();
  }
  if(MenuChange!=0x00){
    CurrentNode=*MenuChange;  //事情都做完了才改變..可以避免事情還沒做完又被切到另一個NODE
    if((WhenExitMenuNode!=0))WhenExitMenuNode();  //先執行離開NODE
    if(MenuChange->OnEnterNode!=0){
      MenuChange->OnEnterNode();//若有進入節點所引發的事件..從這執行
    }
    WhenExitMenuNode=MenuChange->OnExitNode;
    
    Beep1Sec();
    Draw_BG_Image();
  }else{
    ShortBeepSetTimes(3);
  }
}

void ButtonPress(){
  MenuNode *MenuChange;
  switch(NowTask.NodeType){
    case ISRMSG_LButtonPress:
      Check_RDMode('L');
      MenuChange=(MenuNode*)CurrentNode.ButtonLeft;
      break;
    case ISRMSG_RButtonPress: 
      Check_RDMode('R');
      MenuChange=(MenuNode*)CurrentNode.ButtonRight;
      break;
    case ISRMSG_CButtonPress: 
      Check_RDMode('C');
      MenuChange=(MenuNode*)CurrentNode.ButtonEnter; 
      break;
  }
  CheckButtonRelease(MenuChange);
  SetDisCount(AutoShutdownTimeout,Reset);   //按鍵後閑置6分鐘之後RESET(自動關機)
}

void ButtonLongPress(){
  MenuNode *MenuChange;
  switch(NowTask.NodeType){
    case ISRMSG_LButtonLongPress: MenuChange=(MenuNode*)CurrentNode.LButtonLeft; break;
    case ISRMSG_RButtonLongPress: MenuChange=(MenuNode*)CurrentNode.LButtonRight; break;
    case ISRMSG_CButtonLongPress: MenuChange=(MenuNode*)CurrentNode.LButtonEnter; break;
  }
  CheckButtonRelease(MenuChange);
  SetDisCount(AutoShutdownTimeout,Reset);   //按鍵後閑置6分鐘之後RESET(自動關機)
}


void Show_DateTime(){
  RTCSaveCount++;
  RTCSaveCount%=RTCSavePerISR;
  if(RTCSaveCount==0){SaveTime();}
  if (CurrentNode.NAME=="MAIN DATETIME"){
    UpdateRTCFromRTCChip();
    if(BattStruct.BattDetectState==0){Open_BattDetect();}
    Draw_BG_Image();
    unsigned char K[11]={'0','1','2','3','4','5','6','7','8','9',':'};   
    DrawBlock(0,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Day / 10)]);
    DrawBlock(10,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Day % 10)]);
    DrawBlock(20,6,8,8,(unsigned char *)OLED_Size8_Dig[11]);
    DrawBlock(30,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Month / 10)]);
    DrawBlock(40,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Month % 10)]);
    DrawBlock(50,6,8,8,(unsigned char *)OLED_Size8_Dig[11]);
    DrawBlock(60,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Year / 10)]);
    DrawBlock(70,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Year % 10)]);
   
    Print(&K[RTC_Obj.Hour / 10],1,24,24,16);				
    Print(&K[RTC_Obj.Hour % 10],1,40,24,16);				
    Print(&K[10],1,56,24,16);				//:
    Print(&K[RTC_Obj.Minute / 10],1,72,24,16);				
    Print(&K[RTC_Obj.Minute % 10],1,88,24,16);			
    Print(&K[RTC_Obj.Second / 10],1,104,32,8);				
    Print(&K[RTC_Obj.Second % 10],1,112,32,8);	
    
    //SHOW電量
    Full_Block(104,0,17,8,0x81);  //塞滿電池格
    if(BattStruct.BattGrid>1)Full_Block(116,0,3,8,0xBD);  //塞滿電池格
    if(BattStruct.BattGrid>2)Full_Block(111,0,3,8,0xBD);  //塞滿電池格
    if(BattStruct.BattGrid>3)Full_Block(106,0,3,8,0xBD);  //塞滿電池格
    Full_Block(122,0,1,8,0xFF);  //塞滿電池格的最後一條
    if(BattStruct.BattGrid==0){
      if(PowerOffAfter5Sec==0)SetDisCount(5,Reset);   //按鍵後閑置6分鐘之後RESET(自動關機)
      Print("LOW BATTERY",11,20,53,8);
      PowerOffAfter5Sec=1;
    }
  }
}


void ShowDateTime_ConnPC(){  
   ClearScreen();
   UpdateRTCFromRTCChip();
   Draw_Screen((unsigned char *)OLED_SHOW_Main);
   
   unsigned char K[11]={'0','1','2','3','4','5','6','7','8','9',':'};   
   DrawBlock(0,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Month / 10)]);
   DrawBlock(10,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Month % 10)]);
   DrawBlock(18,6,8,8,(unsigned char *)OLED_Size8_Dig[11]);
   DrawBlock(26,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Day / 10)]);
   DrawBlock(36,0,8,8,(unsigned char *)OLED_Size8_Dig[(Date_Obj.Day % 10)]);
      
   Print(&K[RTC_Obj.Hour / 10],1,52,0,8);				
   Print(&K[RTC_Obj.Hour % 10],1,60,0,8);				
   Print(&K[10],1,68,0,8);				//:
   Print(&K[RTC_Obj.Minute / 10],1,76,0,8);				
   Print(&K[RTC_Obj.Minute % 10],1,84,0,8);
   
    //SHOW電量
    Full_Block(104,0,17,8,0x81);  //塞滿電池格
    if(BattStruct.BattGrid>1)Full_Block(116,0,3,8,0xBD);  //塞滿電池格
    if(BattStruct.BattGrid>2)Full_Block(111,0,3,8,0xBD);  //塞滿電池格
    if(BattStruct.BattGrid>3)Full_Block(106,0,3,8,0xBD);  //塞滿電池格
    Full_Block(122,0,1,8,0xFF);  //塞滿電池格的最後一條
}
