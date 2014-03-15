#include "../../DriverExport.h"



ButtonStruct ButtonLeft,ButtonRight,ButtonEnter;

/***********************************************************************************************************
;          P2SetButtonIO �@���u��]�w�@��PIN..N�ӫ��s�n�ŧiN��BUTTON
;   Button(BUTTON����)  En(0�����A1�}��)   PortNum(���w�ĴX��bit)  PortSetting(Ĳ�o�覡..���t0�t�t1)
;   fP(�u���ӫ��s�᪺FUNTION POINT)      fLP(�����ӫ��s�᪺FUNTION POINT)
************************************************************************************************************/
void P2SetButtonIO(ButtonStruct *Button,unsigned char En,unsigned char PortNum,unsigned char PortSetting,void (*fP)(),void (*fLP)()){ 
  P2SEL&=~PortNum;   //�N�S��\������
  P2DIR&=~PortNum;   //�]�w����J
  //-------------------------------------�]�w�ӫ��s�O�_�n�P�त�_------------------------
  if(En==1){
    P2IE|=PortNum;      //�P�त�_
  }else{
    P2IE&=~PortNum;     //���त�_
  }
  //---------------------------�]�w���tĲ�o�έt�tĲ�o..���t0�t�t1------------------------
  if(PortNum==PortSetting){ 
    P2IES|=PortNum;   //�t�tĲ�o
  }else{
    P2IES&=~PortNum;  //���tĲ�o
  }
  //------------------------------------ASSIGN�]�w---------------------------------------
  Button->PortSetting=PortSetting;
  Button->PortNum=PortNum;
  Button->Count=0;   //�C�@��PIN�����A�����F�h�[
  Button->Press=fP;   //function pointer will be called after short Press
  Button->LongPress=fLP;   //function pointer will be called after long Press 
  //------------------------------------�]�wTIMER----------------------------------------
}

void CheckButtonPress(ButtonStruct *Button){
  unsigned char NowState;
  if(Button->PortNum==Button->PortSetting){ //�t�tĲ�o
    NowState=(~P2IN)&Button->PortNum;   
  }else{//���tĲ�o
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
  if(Button->PortNum==Button->PortSetting){ //�t�tĲ�o
    NowState=(~P2IN)&Button->PortNum;   
  }else{//���tĲ�o
    NowState=P2IN&Button->PortNum; 
  }  
//---------------------------------------------�P�w���s���A  
  
  if ( NowState==Button->PortNum){//----------------------------------����}----------
    if(Button->Count>0)Button->Count++;    //���ۤ���..�ɶ��֥[
    if (Button->Count>=180){//----------------����
      Button->Count=0;
      SendLongPressToISRQueue(Button);
    }
  }else{//--------------------------------------------------------------------------------��}----------
    if(Button->Count>2){ //-------------�u��(�ܤ֤]�n����@�q�ɶ�..�_�h�����u��)
      SendPressToISRQueue(Button);      
    }
    Button->Count=0;
  }
}

void ButtonDetect(){ //���X��BUTTON..�N�g�b�o..��o��ISR���ɭԷ|�Ӱ���
  CheckButtonPress(&ButtonLeft);
  CheckButtonPress(&ButtonRight);
  CheckButtonPress(&ButtonEnter);
}
