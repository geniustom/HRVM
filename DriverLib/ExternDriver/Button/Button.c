#include "../../DriverExport.h"



ButtonStruct ButtonLeft,ButtonRight,ButtonEnter;

/***********************************************************************************************************
;          P2SetButtonIO 一次只能設定一個PIN..N個按鈕要宣告N個BUTTON
;   Button(BUTTON指標)  En(0關閉，1開啟)   PortNum(指定第幾個bit)  PortSetting(觸發方式..正緣0負緣1)
;   fP(短按該按鈕後的FUNTION POINT)      fLP(長按該按鈕後的FUNTION POINT)
************************************************************************************************************/
void P2SetButtonIO(ButtonStruct *Button,unsigned char En,unsigned char PortNum,unsigned char PortSetting,void (*fP)(),void (*fLP)()){ 
  P2SEL&=~PortNum;   //將特殊功能關閉
  P2DIR&=~PortNum;   //設定為輸入
  //-------------------------------------設定該按鈕是否要致能中斷------------------------
  if(En==1){
    P2IE|=PortNum;      //致能中斷
  }else{
    P2IE&=~PortNum;     //除能中斷
  }
  //---------------------------設定正緣觸發或負緣觸發..正緣0負緣1------------------------
  if(PortNum==PortSetting){ 
    P2IES|=PortNum;   //負緣觸發
  }else{
    P2IES&=~PortNum;  //正緣觸發
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
    NowState=(~P2IN)&Button->PortNum;   
  }else{//正緣觸發
    NowState=P2IN&Button->PortNum; 
  }  
  if ( NowState==Button->PortNum){Button->Count=1;}
  Open_TA(TAButton);
}

void SendLongPressToISRQueue(ButtonStruct *Button){
  //ShortBeepSetTimes(3);
  switch(Button->PortNum){
    case ButtonC:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_CButtonLongPress);
          break;
    case ButtonL:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_LButtonLongPress);
          break;
    case ButtonR:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->LongPress,ISRMSG_RButtonLongPress);
          break;
  }
  Close_TA(TAButton);
}

void SendPressToISRQueue(ButtonStruct *Button){
  //Beep1Sec();
  switch(Button->PortNum){
    case ButtonC:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_CButtonPress);
          break;
    case ButtonL:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_LButtonPress);
          break;
    case ButtonR:
          AddISRMessage(&ISRMSG,ISRPIRORITY_Button,Button->Press,ISRMSG_RButtonPress);
          break;
  }
  Close_TA(TAButton);
}

void CheckButton(ButtonStruct *Button){
  unsigned char NowState;
  if(Button->PortNum==Button->PortSetting){ //負緣觸發
    NowState=(~P2IN)&Button->PortNum;   
  }else{//正緣觸發
    NowState=P2IN&Button->PortNum; 
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
  CheckButtonPress(&ButtonLeft);
  CheckButtonPress(&ButtonRight);
  CheckButtonPress(&ButtonEnter);
}
