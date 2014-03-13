#include "MenuUI.h"
#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"

unsigned char NowIsAtRDMode=0;
unsigned char ButtonPressList[6];
unsigned char ButtonPressIndex=0;
const unsigned char RDModeList[6]={'L','L','R','R','L','R'};
const MenuNode QC_LCD,To_USERMODE;//*QC_Main;
const MenuNode LCD_TEST_Main;
const MenuNode FR_Main,FR_TEST,FR_OK,FR_FAIL;
const MenuNode DELUser_Main,DELUser_Run,DELUser_OK;

void Erase_UserIndex(){
/*
  for (int j=0; j<6 ;j++){
    BlockErase((64 + j) * 32,1);            // Clear User Test Index Data  
  }
*/
 BlockErase(2048,1);            // Clear User Test Index Data  
}

unsigned char TestFlashRomAt(unsigned long addr){  //測試指定PAGE的FLASHROM是否可正常讀寫
  unsigned int i;
  unsigned char TestOK=1; //先設為成功
  //Draw_BG_Image();
  if (addr%32==0){
    BlockErase(addr,1);
  }
  for(i=0;i<512;i++){USB[i]=i;} //存入
  FSave_Data(addr,USB);
  for(i=0;i<512;i++){USB[i]=0;} //青空
  
  FRead_Data(addr,USB); //讀取

  for(i=0;i<512;i++){
    if((USB[i])!=(i%256))TestOK=0; //讀出來之後如果不對，TestOK=0表示測試失敗
  }
  return TestOK;
}


void CheckFlashRom(){
  _DINT();  
  unsigned char AllTestOK=1;
   
  for(unsigned long i=128;i<256;i++){ //這個RANGE是用來確認UPDATE時皆可正常讀寫
     if(TestFlashRomAt(i)==0)AllTestOK=0; //一有測試不OK的，就回傳失敗
  }
  for(unsigned char i=0;i<5;i++){ //這個RANGE是用來確認每個使用者的第一個儲存PAGE是否可正常讀寫
     if(TestFlashRomAt(UserStartPage[i])==0)AllTestOK=0; //一有測試不OK的，就回傳失敗
  }  
  
  if(AllTestOK==0){
    CurrentNode=FR_FAIL;
  }else{
    CurrentNode=FR_OK;
  }
  ClearScreen();
  Draw_BG_Image();
  _EINT();
}

void ToUserMode(){
  NowIsAtRDMode=0;
  ClearScreen();
}

void DelUserData(){
  _DINT();
  /*
  for(unsigned long i=UserStartPage[0];i<65536;i++){
    if(i%32==0)BlockErase(i,1);
  }
  */
  BlockErase(2048,1);            // Clear User Test Index Data  
  CurrentNode=DELUser_OK;
  ClearScreen();
  Draw_BG_Image();
  
  _EINT();
}

void Check_RDMode(unsigned char btn_val){
   ButtonPressList[ButtonPressIndex]=btn_val;
   ButtonPressIndex++;

   unsigned char Same=1;
   for(int i=0;i<ButtonPressIndex;i++) 
     if(ButtonPressList[i]!=RDModeList[i])Same=0; //比較KEY SQUENCE
   if (Same==0){
     ButtonPressIndex=0; //一有不一樣..就重新開始算起
     return;             //離開
   }
//-------------------------------------------------------  
   if((ButtonPressIndex==6)){ 
      ButtonPressIndex=0;    
      if((CurrentNode.NAME==Main_ECG.NAME)||(NowIsAtRDMode==1)){ 
        //因為從TIME按了 左左右右左右 之後..仍會回到TIME..
        //判斷NowIsAtRDMode是因為切換回來就不用在考慮了
        NowIsAtRDMode^=1;
        if(CurrentNode.OnExitNode!=0x00)CurrentNode.OnExitNode();
        
        if(NowIsAtRDMode==1){
          CurrentNode=QC_LCD;  //切換至RDMODE
        } else if(NowIsAtRDMode==0 ){
           //切換至USERMODE
        }
        if(CurrentNode.OnEnterNode!=0x00)CurrentNode.OnEnterNode();
        ClearScreen();
        Draw_BG_Image();
      }
    }
}



//==================================================================
//----------------------------------------------RD Mode Main Page  
  const MenuNode To_USERMODE={(unsigned int*)&DELUser_Main,(unsigned int*)&QC_LCD,(unsigned int*)&Main_DateTime,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,&ToUserMode,"TO USER",CleenScreen,RDModeNode|StaticScreen};
//----------------------------------------------QC_LCD---OK

  const MenuNode QC_LCD={(unsigned int*)&To_USERMODE,(unsigned int*)&FR_Main,(unsigned int*)&LCD_TEST_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"QC LCD",CleenScreen,RDModeNode|StaticScreen};    
//---------------------------------------------LCD TEST Main---OK

  const MenuNode LCD_TEST_Main={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&QC_LCD,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &FullScreen,0,"LCD TEST",CleenScreen,StaticScreen|LCDTEST};       
//--------------------------------------------FLASH ROM TEST  
   const MenuNode FR_Main={(unsigned int*)&QC_LCD,(unsigned int*)&DELUser_Main,(unsigned int*)&FR_TEST,  //測試前主畫面
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"ROMTEST",CleenScreen,RDModeNode|StaticScreen}; 
  
   const MenuNode FR_TEST={(unsigned int*)0,(unsigned int*)0,(unsigned int*)0, //測試中..
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &CheckFlashRom,0,"WAIT RW",CleenScreen,RDModeNode|StaticScreen}; 
   
   const MenuNode FR_OK={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&FR_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"RW OK",CleenScreen,RDModeNode|StaticScreen}; 

   const MenuNode FR_FAIL={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&FR_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"RW FAIL",CleenScreen,RDModeNode|StaticScreen}; 
   
   //-------------------------------------------------CLEAR FLASH DATA
   const MenuNode DELUser_RUN={(unsigned int*)0,(unsigned int*)0,(unsigned int*)0, //測試中..
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &DelUserData,0,"WAITDEL",CleenScreen,RDModeNode|StaticScreen};
   
   const MenuNode DELUser_Main={(unsigned int*)&FR_Main,(unsigned int*)&To_USERMODE,(unsigned int*)&DELUser_RUN,  //測試前主畫面
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"DELDATA",CleenScreen,RDModeNode|StaticScreen}; 
   
   const MenuNode DELUser_OK={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&DELUser_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"DEL OK",CleenScreen,RDModeNode|StaticScreen}; 
   
   //-----------------------------------------------------------------Check UART
   
