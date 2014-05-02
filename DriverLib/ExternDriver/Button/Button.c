#include "../../DriverExport.h"

#define BUTTON_PxSEL      P1SEL
#define BUTTON_PxDIR      P1DIR
#define BUTTON_PxIE       P1IE
#define BUTTON_PxIES      P1IES
#define BUTTON_PxIFG      P1IFG
#define BUTTON_PxIN       P1IN

ButtonStruct ButtonUp,ButtonDown,ButtonLeft,ButtonRight,ButtonCenter;

/***********************************************************************************************************
;          PxSetButtonIO 一次只能設定一個PIN..N個按鈕要宣告N個BUTTON
;   Button(BUTTON指標)  En(0關閉，1開啟)   PortNum(指定第幾個bit)  PortSetting(觸發方式..正緣0負緣1)
;   fP(短按該按鈕後的FUNTION POINT)      fLP(長按該按鈕後的FUNTION POINT)
************************************************************************************************************/
void PxSetButtonIO(ButtonStruct *Button,unsigned char En,unsigned char PortNum,unsigned char PortSetting,void (*fP)(),void (*fLP)()){ 
  BUTTON_PxSEL&=~PortNum;   //將特殊功能關閉
  BUTTON_PxDIR&=~PortNum;   //設定為輸入
  //-------------------------------------設定該按鈕是否要致能中斷------------------------
  if(En==1){
    BUTTON_PxIE|=PortNum;      //致能中斷
  }else{
    BUTTON_PxIE&=~PortNum;     //除能中斷
  }
  //---------------------------設定正緣觸發或負緣觸發..正緣0負緣1------------------------
  if(PortNum==PortSetting){ 
    BUTTON_PxIES|=PortNum;   //負緣觸發
  }else{
    BUTTON_PxIES&=~PortNum;  //正緣觸發
  }
  //------------------------------------ASSIGN設定---------------------------------------
  Button->PortSetting=PortSetting;
  Button->PortNum=PortNum;
  Button->Count=0;   //每一隻PIN的狀態維持了多久
  Button->Press=fP;   //function pointer will be called after short Press
  Button->LongPress=fLP;   //function pointer will be called after long Press 
  //------------------------------------設定TIMER----------------------------------------
}

void CheckButtonPress(ButtonStruct *Button){
  unsigned char NowState;
  if(Button->PortNum==Button->PortSetting){ //負緣觸發
    NowState=(~BUTTON_PxIN)&Button->PortNum;   
  }else{//正緣觸發
    NowState=BUTTON_PxIN&Button->PortNum; 
  }  
  if ( NowState==Button->PortNum){Button->Count=1;}
  Open_TA(TAButton);
}

void SendLongPressToISRQueue(ButtonStruct *Button){
  //ShortBeepSetTimes(3);
  switch(Button->PortNum){
    case ButtonU:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_UButtonLongPress);
          break;
    case ButtonD:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_DButtonLongPress);
          break;
    case ButtonL:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_LButtonLongPress);
          break;
    case ButtonR:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_RButtonLongPress);
          break;
    case ButtonC:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_CButtonLongPress);
          break;
  }
  Close_TA(TAButton);
}

void SendPressToISRQueue(ButtonStruct *Button){
  //Beep1Sec();
  switch(Button->PortNum){
    case ButtonU:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_UButtonPress);
          break;
    case ButtonD:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_DButtonPress);
          break;
    case ButtonL:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_LButtonPress);
          break;
    case ButtonR:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_RButtonPress);
          break;
    case ButtonC:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_CButtonPress);
          break;
  }
  Close_TA(TAButton);
}

void CheckButton(ButtonStruct *Button){
  unsigned char NowState;
  if(Button->PortNum==Button->PortSetting){ //負緣觸發
    NowState=(~BUTTON_PxIN)&Button->PortNum;   
  }else{//正緣觸發
    NowState=BUTTON_PxIN&Button->PortNum; 
  }  
//---------------------------------------------判定按鈕狀態  
  
  if ( NowState==Button->PortNum){//----------------------------------未放開----------
    if(Button->Count>0)Button->Count++;    //按著不放..時間累加
    if (Button->Count>=180){//----------------長按
      Button->Count=0;
      SendLongPressToISRQueue(Button);
    }
  }else{//--------------------------------------------------------------------------------放開----------
    if(Button->Count>2){ //-------------短按(至少也要持續一段時間..否則視為彈跳)
      SendPressToISRQueue(Button);      
    }
    Button->Count=0;
  }
}

void ButtonDetect(){ //有幾個BUTTON..就寫在這..當發生ISR的時候會來偵測
  CheckButtonPress(&ButtonUp);
  CheckButtonPress(&ButtonDown);
  CheckButtonPress(&ButtonLeft);
  CheckButtonPress(&ButtonRight);
  CheckButtonPress(&ButtonCenter);
}
