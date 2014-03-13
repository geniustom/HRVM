#include "Nand.h"
#include "../DriverExport.h"

void Init_Flash(void){
  FLASHBus = 0;
  FLASHBus |= CLE;    //設置CLE為輸出管腳 P3.0
  FLASHBus |= CE;    //設置CE~為輸出管腳 P3.2
  FLASHBus |= ALE;    //設置ALE為輸出管腳 P3.1
  FLASHBus |= RE;    //設置RE~為輸出管腳 P3.3
  FLASHBus |= WE;    //設置WE~為輸出管腳 P2.4
  FLASHBus |= WP;    //設置WP為輸出管腳 P2.5
  FLASHControl |= WP;    
  RBPort &= ~RB; //設置R/B為輸入管腳 P2.7
}

void Reset_Flash(void){ 
  FLASHDIR = 0xff;			// 設置P1口為輸出方向
  CE_Enable;
    CLE_Enable;
      WE_Enable;
        FLASHDATAOUT = 0xff;			// 輸出RESET控制命令
      WE_Disable;
    CLE_Disable;
    
    while(RBIO&RB==0)_NOP();
  CE_Disable;
}

void EraseAddress(unsigned long nAddr){
  unsigned char nADD1 = nAddr & 0x00ff;
  unsigned char nADD2 = (nAddr >> 8) & 0x00ff;
  unsigned char nADD3 = (nAddr >> 16) & 0x00ff;
  
  //while(RBIO&RB==0)_NOP();
  
  ALE_Enable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD1);
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD2);
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD3);
    WE_Disable;
  ALE_Disable;
}

void AccessAddress(unsigned char nCol,unsigned long nRow){
  unsigned char nADD1 = nRow & 0x00ff;
  unsigned char nADD2 = (nRow >> 8) & 0x00ff;
  unsigned char nADD3 = (nRow >> 16) & 0x00ff; 
  
  while(RBIO&RB==0)_NOP();
  
  ALE_Enable;  
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nCol);//送行位址
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD1);//送列位址BYTE1
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD2);//送列位址BYTE2
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD3);//送列位址BYTE3
    WE_Disable;
  ALE_Disable;
}

void SendFlashCMD(unsigned char CMD){
  CLE_Enable;
    WE_Enable;
      FLASHDATAOUT = CMD;			//輸出塊擦出命令
    WE_Disable;
  CLE_Disable;

}
////////////////////////////////////////
// 正確返回 1，錯誤返回 0
int BlockErase(unsigned long nAddr,unsigned char BusyWait){
  int nTemp = 0;
  FLASHDIR = 0xff;			//設置P1口為輸出方向
  
  CE_Enable;   
    SendFlashCMD(0x60);
    EraseAddress(nAddr); 
    SendFlashCMD(0xd0);
    for(int i = 50;i > 0;i--);
    FLASHDIR = 0x00;				//設置P1口為輸入方向
    RE_Enable;
      nTemp = FLASHDATAIN;
    RE_Disable; 
  CE_Disable;
  
  if(BusyWait==1)while(RBIO&RB==0)_NOP();
  //for(int i=0;i<5000;i++);  //完成之後要DELAY5000..否則寫會有問題
  if(nTemp & 0x01) return 0;
  else return 1;
}


void PageRead(unsigned int nCol,unsigned long nRow,unsigned char *pBuf,unsigned int Len){
  FLASHDIR = 0xff;		//設置P1口為輸出方向
  unsigned char fCMD=(unsigned char)(nCol>>8);
  if(nCol>=512){//存取被用區
    fCMD=0x50;
    nCol-=512;
  }
  
  CE_Enable;  
    SendFlashCMD(fCMD); 
    AccessAddress(nCol,nRow);
    FLASHDIR = 0x00;			//設置P1口為輸入方向
    for(int i = 50;i > 0;i--);
    for(int j = 0;j < Len;j++){
      RE_Enable;
        pBuf[j] = FLASHDATAIN;
      RE_Disable;
    }
  CE_Disable;

  while(RBIO&RB==0)_NOP();
}

////////////////////////////////////////
// 正確返回 1，錯誤返回 0
int PageWrite(unsigned int nCol,unsigned long nRow,unsigned char *pBuf,unsigned int Len){
  int nTemp = 0;
  unsigned char fCMD=(unsigned char)(nCol>>8);
  if(nCol>=512){//存取被用區
    fCMD=0x50;
    nCol-=512;
  }
  FLASHDIR = 0xff;			//設置P1口為輸出方向
  
  CE_Enable;
    SendFlashCMD(fCMD);
    SendFlashCMD(0x80);
    AccessAddress(nCol,nRow);
    for(unsigned int j = 0;j < Len;j++){
      WE_Enable;
        FLASHDATAOUT = pBuf[j];
      WE_Disable;
    }
    SendFlashCMD(0x10);
    for(int i = 50;i > 0;i--);
    SendFlashCMD(0x70);
    FLASHDIR = 0x00;			//設置P1口為輸入方向

    for(unsigned int i= 5000;i > 0;i--){             //如果未READY就持續等待
      RE_Enable;
        nTemp = FLASHDATAIN;
      RE_Disable;
      if(nTemp == 0xc0) break;
    }

  CE_Disable;
  
  while(RBIO&RB==0)_NOP();
  if(nTemp == 0xc0) return 1;
  else return 0;
}


void FSave_Data(unsigned long Page_index,unsigned char *Databuff){
  unsigned char Sparebuff[16]={0};
  ECC_gen(Databuff,Sparebuff);		   		//generator ECC code for main area data	
  S_ECC_gen(Sparebuff,Sparebuff);		   	//generator ECC code for spare area data
  PageWrite(512,Page_index,Sparebuff,PAGE_SPARE_SIZE);  //Write ECC to Spare area
  PageWrite(0,Page_index,Databuff,PAGE_DATA_SIZE);      //Write data to Data area
}

void FRead_Data(unsigned long Page_index,unsigned char *Databuff){
  unsigned char Sparebuff[16]={0};
  unsigned char Sparebuff1[16]={0};
  PageRead(512,Page_index,Sparebuff,PAGE_SPARE_SIZE);  //Read ECC to Spare area

  unsigned char data_buff1,data_buff2;
  data_buff1=Sparebuff[9];
  data_buff2=Sparebuff[10];			   		//set S_ECC to 0
  Sparebuff[9]=0X00;
  Sparebuff[10]=0X00;	
  S_ECC_gen(Sparebuff,Sparebuff1); 		   		//產生S_ECC code為ECC1
  Sparebuff[9]=data_buff1;
  Sparebuff[10]=data_buff2;
  
  PageRead(0,Page_index,Databuff,PAGE_DATA_SIZE);               //讀取資料        
  ECC_gen(Databuff,Sparebuff1); 		   		//產生新碼ECC code為ECC1
  PageRead(512,Page_index,Sparebuff,PAGE_SPARE_SIZE);           //讀取舊ECC code為ECC
}

/*
int WriteByte(unsigned int nCol,unsigned long nRow,unsigned char nValue){
  int nTemp = 0;
  //因為FLASH的行位址是一個BYTE..用寫入扇形區來決定奇數BYTE或偶數BYTE
  unsigned char fCMD=(unsigned char)(nCol>>8);
  if(nCol>=512){//存取被用區
    fCMD=0x50;
    nCol-=512;
  }
  FLASHDIR = 0xff;			//設置P1口為輸出方向 

  CE_Enable;
    SendFlashCMD(fCMD);
    SendFlashCMD(0x80);
    AccessAddress(nCol,nRow);
    WE_Enable;
      FLASHDATAOUT = nValue;
    WE_Disable;
    SendFlashCMD(0x10);
    for(int i = 50;i > 0;i--);
    SendFlashCMD(0x70);
    FLASHDIR = 0x00;			//設置P1口為輸入方向
    for(unsigned int i = 50000;i > 0;i--){  //如果未READY就持續等待
      RE_Enable;
        nTemp = FLASHDATAIN;
      RE_Disable;
      if(nTemp == 0xc0) break;
    }
  CE_Disable;
  
  while(RBIO&RB==0)_NOP();
  if(nTemp == 0xc0) return 1;
  else return 0;  
}


unsigned char ReadByte(unsigned int nCol,unsigned long nRow){
  unsigned char fCMD=(unsigned char)(nCol>>8);
  //因為FLASH的行位址是一個BYTE..用寫入扇形區來決定奇數BYTE或偶數BYTE
  if(nCol>=512){//存取被用區
    fCMD=0x50;
    nCol-=512;
  }
  FLASHDIR = 0xff;		//設置P1口為輸出方向
  
  CE_Enable;
    SendFlashCMD(fCMD);
    AccessAddress(nCol,nRow);
    FLASHDIR = 0x00;		//設置P1口為輸入方向
    for(int i = 50;i > 0;i--);
    RE_Enable;
      unsigned char ReturnData= FLASHDATAIN;
    RE_Disable;
  CE_Disable;
  
  while(RBIO&RB==0)_NOP();
  return ReturnData;
}

*/
/*
  int j=0,i1=0,i2=0;
  i1=Sparebuff[9]^Sparebuff1[9]; 	   			//XOR the ECC code
  i2=Sparebuff[10]^Sparebuff1[10];
  j=i1|i2; //j=0 表示無ERROR
*/  
/*
  int L=0,l1=0,l2=0,l3=0;
  l1=Sparebuff[6]^Sparebuff1[6]; 	   			//XOR the ECC code
  l2=Sparebuff[7]^Sparebuff1[7];
  l3=Sparebuff[8]^Sparebuff1[8];
  L=l1|l2|l3;  //L=0 表示無ERROR
*/

