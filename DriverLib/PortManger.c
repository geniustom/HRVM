#include "DriverExport.h"

void SelectSpecialPins(int portnum){  //設定特殊功能接腳
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1SEL|=ControlLine;break;
    case 2: P2SEL|=ControlLine;break;
    case 3: P3SEL|=ControlLine;break;
    case 4: P4SEL|=ControlLine;break;
    case 5: P5SEL|=ControlLine;break;
    case 6: P6SEL|=ControlLine;break;
  }
}

void SelectGPIOPins(int portnum){  //設定IO功能接腳
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1SEL&=~ControlLine;break;
    case 2: P2SEL&=~ControlLine;break;
    case 3: P3SEL&=~ControlLine;break;
    case 4: P4SEL&=~ControlLine;break;
    case 5: P5SEL&=~ControlLine;break;
    case 6: P6SEL&=~ControlLine;break;
  }
}


void SetGPIOInput(int portnum){   //設定為輸入
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1DIR&=~ControlLine;break;
    case 2: P2DIR&=~ControlLine;break;
    case 3: P3DIR&=~ControlLine;break;
    case 4: P4DIR&=~ControlLine;break;
    case 5: P5DIR&=~ControlLine;break;
    case 6: P6DIR&=~ControlLine;break;
  }
}

void SetGPIOOutput(int portnum){  //設定為輸出
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1DIR|=ControlLine;break;
    case 2: P2DIR|=ControlLine;break;
    case 3: P3DIR|=ControlLine;break;
    case 4: P4DIR|=ControlLine;break;
    case 5: P5DIR|=ControlLine;break;
    case 6: P6DIR|=ControlLine;break;
  }
}

void OutputGPIOHi(int portnum){  //設定輸出HI
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1OUT|=ControlLine;break;
    case 2: P2OUT|=ControlLine;break;
    case 3: P3OUT|=ControlLine;break;
    case 4: P4OUT|=ControlLine;break;
    case 5: P5OUT|=ControlLine;break;
    case 6: P6OUT|=ControlLine;break;
  }
}

void OutputGPIOLo(int portnum){  //設定輸出LO
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1OUT&=~ControlLine;break;
    case 2: P2OUT&=~ControlLine;break;
    case 3: P3OUT&=~ControlLine;break;
    case 4: P4OUT&=~ControlLine;break;
    case 5: P5OUT&=~ControlLine;break;
    case 6: P6OUT&=~ControlLine;break;
  }
}

void OutputGPIO(int portnum){  //設定設什麼就輸出什麼
  char P=portnum>>8;
  char ControlLine=(char)(portnum&0xff);
  switch (P){
    case 1: P1OUT=ControlLine;break;
    case 2: P2OUT=ControlLine;break;
    case 3: P3OUT=ControlLine;break;
    case 4: P4OUT=ControlLine;break;
    case 5: P5OUT=ControlLine;break;
    case 6: P6OUT=ControlLine;break;
  }
}

char GetGPIO(int portnum){  //設定設什麼就輸出什麼
  char P=portnum>>8;
  char Result=0;
  switch (P){
    case 1: Result=P1IN;break;
    case 2: Result=P2IN;break;
    case 3: Result=P3IN;break;
    case 4: Result=P4IN;break;
    case 5: Result=P5IN;break;
    case 6: Result=P6IN;break;
  }
  return Result;
}
