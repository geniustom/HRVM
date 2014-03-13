#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"

CMDListener Uart;
unsigned char CMDBUF[11];
/*************************************************************************
                            CMD  TYPE
    'C','M','D',[CMDtype,Address],[DataLength],[CheckSum]
    ^^  ^^  ^^   ^^^^^^^ ^^^^^^^   ^^^^^^^^^^   ^^^^^^^^
    1   1   1       1       4          2           1       //BYTE count
    0   1   2       3       4~7        8~9         10
*************************************************************************/
void InitUartStruct(unsigned char *CMDBUF,unsigned char *DATABUF){
  Uart.CMDBuf=CMDBUF;
  Uart.DATABuf=DATABUF;
  Uart.CMDIndex=0;
  Uart.DataIndex=0;
  Uart.CMDMode=0;
}

void SendTextToUart(unsigned char *SBUF,unsigned int length){
  for (int i=0;i<length;i++){
    while (!(IFG1 & UTXIFG0));                                   // USART0 TX buffer ready?
    TXBUF0 = SBUF[i];
  }
}

void SendByteToUart(unsigned char SBUF){
    while (!(IFG1 & UTXIFG0));                                   // USART0 TX buffer ready?
    TXBUF0 = SBUF;
}
/*************************************************************************
                          CMD TYPE之格式
  0x00 讀取所送ADDRESS之FLASH
  0x01 ERASE所送ADDRESS之FLASH
  0x02 寫入所送ADDRESS之FLASH
*************************************************************************/
void DoUSBCMD(){
  unsigned char check_sum=0;
  Uart.CMDtodo=0;
  switch(Uart.CMD_Type){
  case 0:
    FRead_Data(Uart.CMD_Address,Uart.DATABuf);
    for(int i=0;i<Uart.Datalength;i++){
      SendByteToUart(Uart.DATABuf[i]);
      check_sum^=Uart.DATABuf[i];
    }
    SendByteToUart(check_sum);
    SendByteToUart(Uart.CheckSum);
    Uart.CMDMode=0;
    break;
  case 1:
    BlockErase(Uart.CMD_Address,0);
    SendTextToUart("ACKOK",5);
    SendByteToUart(Uart.CheckSum);
    Uart.CMDMode=0;
    break;
  case 2:
    if(Uart.CMD_Address%32==0)BlockErase(Uart.CMD_Address,0);
    SendTextToUart("ACKOK",5);
    SendByteToUart(Uart.CheckSum);
    break;
  case 4:
    ShowTimeInFlash();
    SendTextToUart("ACKOK",5);
    SendByteToUart(Uart.CheckSum);
    Uart.CMDMode=0;
    break;
  }
  Uart.CMDIndex=0;
}

  /*******************************************************
    若視為有效CMD..將相關參數設定進去
  *******************************************************/
void ProcessCMD(){
   Uart.Datalength=((unsigned int)Uart.CMDBuf[8])<<8|Uart.CMDBuf[9];
   Uart.CMD_Type=Uart.CMDBuf[3];
   Uart.CMD_Address=((unsigned long)Uart.CMDBuf[4])<<24  |
                    ((unsigned long)Uart.CMDBuf[5])<<16  |
                    ((unsigned long)Uart.CMDBuf[6])<<8   |
                    ((unsigned long)Uart.CMDBuf[7]);
   Uart.CMDtodo=1;      //OS判定為HI時..開始做該做的事
}



void CheckCMDMode(unsigned char RxData){
  Uart.CMDBuf[Uart.CMDIndex]=RxData;
  unsigned char ThisIsNotCMD=0,CMDCheckSum=0;
  switch(Uart.CMDIndex){
  case 0:
    if(RxData!='C')ThisIsNotCMD=1;break;
  case 1:
    if(RxData!='M')ThisIsNotCMD=1;break;
  case 2:
    if(RxData!='D')ThisIsNotCMD=1;break;   
  case CMDSize:
    for(int i=0;i<CMDSize;i++){
      CMDCheckSum^=Uart.CMDBuf[i];
    }
    if(CMDCheckSum!=Uart.CMDBuf[CMDSize]){
      ThisIsNotCMD=1;
    }else{
      Uart.CheckSum=CMDCheckSum;
      Uart.DataIndex=0;
      ProcessCMD();
      Uart.CMDMode=1;
    }
  }
  
  if(ThisIsNotCMD==1){
    Uart.CMDIndex=0;
    return;
  }
  Uart.CMDIndex++;
}

void CheckDataMode(unsigned char RxData){
  Uart.DATABuf[Uart.DataIndex]=RxData;
  Uart.DataIndex++;
  unsigned char checksum=0;
  if(Uart.DataIndex==Uart.Datalength){  //儲存完成
    for(int i=0;i<Uart.Datalength;i++){
      checksum^=Uart.DATABuf[i];
    }
    for(int i=Uart.Datalength;i<512;i++){Uart.DATABuf[i]=0xff;}
    FSave_Data(Uart.CMD_Address,Uart.DATABuf);
    Uart.CMDMode=0;
    Uart.CMDIndex=0;
    Uart.DataIndex=0;
    SendTextToUart("ACKOK",5);
    SendByteToUart(checksum);
    SendByteToUart(Uart.CheckSum);
  }
}

void DoNormalRx(unsigned char RxData){
  if(Uart.CMDMode==0){
    CheckCMDMode(RxData);
  }else{
    CheckDataMode(RxData);
  }
}


