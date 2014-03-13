#include "AppExport.h"

void Init_Button(){
  P2SetButtonIO(&ButtonLeft,1,ButtonL,ButtonL,ButtonPress,ButtonLongPress);
  P2SetButtonIO(&ButtonRight,1,ButtonR,ButtonR,ButtonPress,ButtonLongPress);
  P2SetButtonIO(&ButtonEnter,1,ButtonC,ButtonC,ButtonPress,ButtonLongPress);  
}

void BackGroundTask(){
  StepSave_Flash(&ECGRawFile);
  if(ECGStruct.ECGFinal==1&&ECGRawFile.Busy==0){
    ECGFinal();
  }
}

void BackGroundUSBTask(){
  ShowDateTime_ConnPC();
  SetDisCount(0,0); //�קK�۰�����
  if(Uart.CMDtodo==1){
    DoUSBCMD();
  }
  if((P2IN&USBPIN)==0){  //�ޱ���
    PowerDownLongTime();
  }
  //USB_CONNECT();
}


void Run_Main(){
  Init_Devicelib();
  Init_Device(Init_Button);
  InitUartStruct(CMDBUF,USB);
  AddGPIOHook(USBPIN,1,Reset,0);  //����USB�NRESET
  //AddGPIOHook(USBPIN,3,PowerDownLongTime,0);  //����USB�NRESET
  Init_RTOS();
  Init_RTC();
  Start_RTC(Show_DateTime);
  LoadTime();
  Beep1Sec();
  P2DIR&=~USBPIN;
  //P5DIR|=BIT5;  P5OUT|=BIT5;   //USB Reset pin pull hi
  if((P2IN&USBPIN)!=0){
    if(BattStruct.BattDetectState==0){Open_BattDetect();}
    //AddGPIOHook(USBPIN,2,PowerDown,0);  //����USB�NRESET
    Run_RTOS(BackGroundUSBTask,0,ShowPIC1Page,10);
  }else{
    PowerOffAfter5Sec=0;
    //CurrentNode=Standby;
    //PowerDown();  //�{���@��l�|���b�o
    CurrentNode=Main_DateTime;
    PowerOn();
    Run_RTOS(BackGroundTask,0,ShowPIC1Page,10);
  }
}




void main( void ){
  BootLoaderMain(Run_Main,DoNormalRx);
}

