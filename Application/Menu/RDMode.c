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

unsigned char TestFlashRomAt(unsigned long addr){  //���ի��wPAGE��FLASHROM�O�_�i���`Ū�g
  unsigned int i;
  unsigned char TestOK=1; //���]�����\
  //Draw_BG_Image();
  if (addr%32==0){
    BlockErase(addr,1);
  }
  for(i=0;i<512;i++){USB[i]=i;} //�s�J
  FSave_Data(addr,USB);
  for(i=0;i<512;i++){USB[i]=0;} //�C��
  
  FRead_Data(addr,USB); //Ū��

  for(i=0;i<512;i++){
    if((USB[i])!=(i%256))TestOK=0; //Ū�X�Ӥ���p�G����ATestOK=0��ܴ��ե���
  }
  return TestOK;
}


void CheckFlashRom(){
  _DINT();  
  unsigned char AllTestOK=1;
   
  for(unsigned long i=128;i<256;i++){ //�o��RANGE�O�ΨӽT�{UPDATE�ɬҥi���`Ū�g
     if(TestFlashRomAt(i)==0)AllTestOK=0; //�@�����դ�OK���A�N�^�ǥ���
  }
  for(unsigned char i=0;i<5;i++){ //�o��RANGE�O�ΨӽT�{�C�ӨϥΪ̪��Ĥ@���x�sPAGE�O�_�i���`Ū�g
     if(TestFlashRomAt(UserStartPage[i])==0)AllTestOK=0; //�@�����դ�OK���A�N�^�ǥ���
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
     if(ButtonPressList[i]!=RDModeList[i])Same=0; //���KEY SQUENCE
   if (Same==0){
     ButtonPressIndex=0; //�@�����@��..�N���s�}�l��_
     return;             //���}
   }
//-------------------------------------------------------  
   if((ButtonPressIndex==6)){ 
      ButtonPressIndex=0;    
      if((CurrentNode.NAME==Main_ECG.NAME)||(NowIsAtRDMode==1)){ 
        //�]���qTIME���F �����k�k���k ����..���|�^��TIME..
        //�P�_NowIsAtRDMode�O�]�������^�ӴN���Φb�Ҽ{�F
        NowIsAtRDMode^=1;
        if(CurrentNode.OnExitNode!=0x00)CurrentNode.OnExitNode();
        
        if(NowIsAtRDMode==1){
          CurrentNode=QC_LCD;  //������RDMODE
        } else if(NowIsAtRDMode==0 ){
           //������USERMODE
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
   const MenuNode FR_Main={(unsigned int*)&QC_LCD,(unsigned int*)&DELUser_Main,(unsigned int*)&FR_TEST,  //���իe�D�e��
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"ROMTEST",CleenScreen,RDModeNode|StaticScreen}; 
  
   const MenuNode FR_TEST={(unsigned int*)0,(unsigned int*)0,(unsigned int*)0, //���դ�..
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &CheckFlashRom,0,"WAIT RW",CleenScreen,RDModeNode|StaticScreen}; 
   
   const MenuNode FR_OK={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&FR_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"RW OK",CleenScreen,RDModeNode|StaticScreen}; 

   const MenuNode FR_FAIL={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&FR_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"RW FAIL",CleenScreen,RDModeNode|StaticScreen}; 
   
   //-------------------------------------------------CLEAR FLASH DATA
   const MenuNode DELUser_RUN={(unsigned int*)0,(unsigned int*)0,(unsigned int*)0, //���դ�..
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         &DelUserData,0,"WAITDEL",CleenScreen,RDModeNode|StaticScreen};
   
   const MenuNode DELUser_Main={(unsigned int*)&FR_Main,(unsigned int*)&To_USERMODE,(unsigned int*)&DELUser_RUN,  //���իe�D�e��
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"DELDATA",CleenScreen,RDModeNode|StaticScreen}; 
   
   const MenuNode DELUser_OK={(unsigned int*)0,(unsigned int*)0,(unsigned int*)&DELUser_Main,
                         (unsigned int*)0,(unsigned int*)0,(unsigned int*)0,
                         0,0,"DEL OK",CleenScreen,RDModeNode|StaticScreen}; 
   
   //-----------------------------------------------------------------Check UART
   
