#include "Nand.h"
#include "../DriverExport.h"

 unsigned char LP0=0,LP1=0,LP2=0,LP3=0,LP4=0,LP5=0,LP6=0,LP7=0,LP8=0,LP9=0,LP10=0,LP11=0,LP12=0,LP13=0,LP14=0,LP15=0,LP16=0,LP17=0;
 unsigned char CP0=0,CP1=0,CP2=0,CP3=0,CP4=0,CP5=0;
 
void WriteByteToFlashBuf(FlashStruct *F,unsigned char Byte){
  if(F->Busy==0){//------------------MainBuf未滿..存入MainBuf
    if(F->BackupBufIndex>0){//先把備用區的DATA塞進去..再把備用區清空
      for(int i=0;i<F->BackupBufIndex;i++){
        F->MainBuf[F->MainBufIndex]=F->BackupBuf[i];
        F->MainBufIndex++;
      }
      F->BackupBufIndex=0;
      F->AccessPage++;
    }
    F->MainBuf[F->MainBufIndex]=Byte;
    F->MainBufIndex++;
    if(F->MainBufIndex>=512){
      F->NeedtoAccess=2;
      F->Busy=1;
    }
  }else{//------------------MainBuf以滿..存入BackupBuf
    if(F->BackupBufIndex>=F->BackLen)AddErrorCode(FlashBufFull);  //BACKUP BUF FULL
    F->BackupBuf[F->BackupBufIndex]=Byte;
    F->BackupBufIndex++;
  }
}
void ECC_genStart(FlashStruct *F){
 LP0=0;LP1=0;LP2=0;LP3=0;LP4=0;LP5=0;LP6=0;LP7=0;LP8=0;LP9=0;LP10=0;LP11=0;LP12=0;LP13=0;LP14=0;LP15=0;LP16=0;LP17=0;
 CP0=0;CP1=0;CP2=0;CP3=0;CP4=0;CP5=0;
}

void ECC_genEND(FlashStruct *F){
 F->Sparebuff[6]=LP0|LP1|LP2|LP3|LP4|LP5|LP6|LP7;
 F->Sparebuff[7]=LP8|LP9|LP10|LP11|LP12|LP13|LP14|LP15;
 F->Sparebuff[8]=LP16|LP17|CP0|CP1|CP2|CP3|CP4|CP5;
 F->SpareIndex=0;
 F->NowStep=2;
}

void Step_ECC_gen(FlashStruct *F){
  unsigned int index=F->SpareIndex;
  if (index==0){ECC_genStart(F);}
  else if(index>=512){ECC_genEND(F); return;}
  unsigned char data,checksum,bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7;
   data=F->MainBuf[F->SpareIndex];
   bit0=data&(0x01);
   bit1=data&(0x02);
   bit2=data&(0x04);
   bit3=data&(0x08);
   bit4=data&(0x10);
   bit5=data&(0x20);
   bit6=data&(0x40);
   bit7=data&(0x80);
 
   checksum=data; //因為每個BIT再做XOR是多此一舉
   //checksum=bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0;
   
   if (index&0x01){LP1^=checksum;}else{LP0^=checksum;}
   if (index&0x02){LP3^=checksum;}else{LP2^=checksum;}
   if (index&0x04){LP5^=checksum;}else{LP4^=checksum;}
   if (index&0x08){LP7^=checksum;}else{LP6^=checksum;}
   if (index&0x10){LP9^=checksum;}else{LP8^=checksum;}
   if (index&0x20){LP11^=checksum;}else{LP10^=checksum;}
   if (index&0x40){LP13^=checksum;}else{LP12^=checksum;}
   if (index&0x80){LP15^=checksum;}else{LP14^=checksum;}
   if (index&0xA0){LP17^=checksum;}else{LP16^=checksum;}
   CP0^=(bit6^bit4^bit2^bit0);  //4次XOR
   CP1^=(bit7^bit5^bit3^bit1);  //4次XOR
   CP2^=(bit5^bit4^bit1^bit0);  //4次XOR
   CP3^=(bit7^bit6^bit3^bit2);  //4次XOR
   CP4^=(bit3^bit2^bit1^bit0);  //4次XOR
   CP5^=(bit7^bit6^bit5^bit4);  //4次XOR
   F->SpareIndex++;
}
 
void S_ECC_genEND(FlashStruct *F){
 F->Sparebuff[9]=LP0|LP1|LP2|LP3|CP0|CP1|CP2|CP3;
 F->Sparebuff[10]=CP4|CP5|0XFC;
 F->SpareIndex=0;
 F->NowStep=2;
}

void Step_S_ECC_gen(FlashStruct *F){
  unsigned int index=F->SpareIndex;
  if (index==0){ECC_genStart(F);}
  else if(index>=512){ECC_genEND(F); return;}
  unsigned char checksum,data,bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7;
  for(int i=0;i<16;i++) {
    data=F->MainBuf[F->SpareIndex];
    bit0=data&(0x01);
    bit1=data&(0x02);
    bit2=data&(0x04);
    bit3=data&(0x08);
    bit4=data&(0x10);
    bit5=data&(0x20);
    bit6=data&(0x40);
    bit7=data&(0x80);
    checksum=data; //因為每個BIT再做XOR是多此一舉
   //checksum=bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0;
   checksum=bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0;
   if (i&0x01){LP1^=checksum;}else{LP0^=checksum;}
   if (i&0x02){LP3^=checksum;}else{LP2^=checksum;}
   CP0^=(bit6^bit4^bit2^bit0);
   CP1^=(bit7^bit5^bit3^bit1);
   CP2^=(bit5^bit4^bit1^bit0);
   CP3^=(bit7^bit6^bit3^bit2);
   CP4^=(bit3^bit2^bit1^bit0);
   CP5^=(bit7^bit6^bit5^bit4);	
 }
}

void InitFlashStruct(FlashStruct *F,unsigned char *MainBuf,unsigned char *BackupBuf,unsigned int BackLen){
  F->Busy=0;           //busy for RW
  F->NeedtoAccess=0;   //0:don't use 1:Read  2:Write
  F->NowStep=0;         //if now is busy then do RW by step
  F->MainBuf=MainBuf;       //if MainBuf not full..save data to MainBuf 
  F->MainBufIndex=0;
  F->BackupBuf=BackupBuf;     //if MainBuf full..the MainBuf locked.. and save data to BackupBuf
  F->BackupBufIndex=0;
  F->BackLen=BackLen;
  F->SpareIndex=0;
  F->AccessPage=0;
  for(int i=0;i<PAGE_SPARE_SIZE;i++){F->Sparebuff[i]=0;}
}

void PageWriteStart(unsigned int nCol,unsigned long nRow){
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
}

void PageWriteEnd(){
    int nTemp = 0;
    SendFlashCMD(0x10);
    for(int i = 50;i > 0;i--);
    SendFlashCMD(0x70);
    FLASHDIR = 0x00;			//設置P1口為輸入方向
    for(unsigned int i= 50000;i > 0;i--){             //如果未READY就持續等待
      RE_Enable;
        nTemp = FLASHDATAIN;
      RE_Disable;
      if(nTemp == 0xc0) break;
    }
  CE_Disable;
}

void PageReadStart(unsigned int nCol,unsigned long nRow){
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
}

void PageReadEnd(){
  CE_Disable;
}

void StepSave_Flash(FlashStruct *F){
  if(RBIO&RB==0)return;
  if(F->NeedtoAccess!=2)return;
  switch(F->NowStep){
  case 0:
    //if(F->AccessPage%32==0)BlockErase(F->AccessPage,0);  //MARK掉這行..速度較快..但須作額外的ERASE
    for(int i=0;i<PAGE_SPARE_SIZE;i++){F->Sparebuff[i]=0;}
    F->NowStep=1;    break;
  case 1:
    //ECC_gen(F->MainBuf,F->Sparebuff);	        //generator ECC code for main area data
    for(int i=0;i<8;i++)Step_ECC_gen(F);
  case 2:
    for(int i=0;i<8;i++)Step_S_ECC_gen(F);
    //S_ECC_gen(F->Sparebuff,F->Sparebuff);	//generator ECC code for spare area data
    F->NowStep=3;    break;    
  case 3:
    PageWrite(512,F->AccessPage,F->Sparebuff,PAGE_SPARE_SIZE);  //Write ECC to Spare area 
    F->NowStep=4;    break;
  case 516:
    PageWriteEnd();
    F->NeedtoAccess=0;
    F->NowStep=0;
    F->MainBufIndex=0;
    F->Busy=0;
    break;
  case 4:
    PageWriteStart(0,F->AccessPage);
  default:
    for(int i=0;i<128;i++){
      WE_Enable;
        FLASHDATAOUT = F->MainBuf[F->NowStep-4];
      WE_Disable;
      F->NowStep++;
    }
  }
}


/*
void StepSave_Flash(FlashStruct *F){
  if(RBIO&RB==0)return;
  if(F->NeedtoAccess!=2)return;
  if(F->Busy==0)return;
  switch(F->NowStep){
  case 0:
    if(F->AccessPage%32==0)BlockErase(F->AccessPage,0);
    for(int i=0;i<PAGE_SPARE_SIZE;i++){F->Sparebuff[i]=0;}
    F->NowStep=1;    break;
  case 1:
    //ECC_gen(F->MainBuf,F->Sparebuff);	        //generator ECC code for main area data
    for(int i=0;i<16;i++)Step_ECC_gen(F); //作一次要20us..共做16次
  case 2:
    //for(int i=0;i<8;i++)Step_S_ECC_gen(F);//作一次要20us..共做4次
    S_ECC_gen(F->Sparebuff,F->Sparebuff);	//generator ECC code for spare area data
    F->NowStep=3;    break;    
  case 3:
    PageWrite(512,F->AccessPage,F->Sparebuff,PAGE_SPARE_SIZE);  //Write ECC to Spare area 
    F->NowStep=4;    break;
  case 516:
    PageWriteEnd();
    F->NeedtoAccess=0;
    F->NowStep=0;
    F->MainBufIndex=0;
    F->Busy=0;
    break;
  case 4:
    PageWriteStart(0,F->AccessPage);
  default:
    for(int i=0;i<128;i++){  //作一次要4us..共做32次
      WE_Enable;
        FLASHDATAOUT = F->MainBuf[F->NowStep-4];
      WE_Disable;
      F->NowStep++;
    }
  }
}
*/


void StepRead_Flash(FlashStruct *F){
  if(RBIO&RB==0)return;
  if(F->NeedtoAccess!=1)return;
  switch(F->NowStep){
  case 0:
    PageRead(512,F->AccessPage,F->Sparebuff,PAGE_SPARE_SIZE);  //Read ECC to Spare area
    F->NowStep=1;
    break;
  case 1:
    LP16=F->Sparebuff[9];
    LP17=F->Sparebuff[10];			   		//set S_ECC to 0
    F->Sparebuff[9]=0X00;
    F->Sparebuff[10]=0X00;	
    Step_S_ECC_gen(F);
    F->Sparebuff[9]=LP16;
    F->Sparebuff[10]=LP17;
    F->NowStep=2;
    break;
  case 514:
    PageReadEnd();
    Step_ECC_gen(F);
    F->NeedtoAccess=0;
    F->NowStep=0;
    //ECC_gen(Databuff,Sparebuff1); 		   		//產生新碼ECC code為ECC1
    break;
  case 2:
    PageReadStart(0,F->AccessPage);
  default:
    RE_Enable;
      F->MainBuf[F->NowStep-2] = FLASHDATAIN;
    RE_Disable;
    F->NowStep++;
  }
}


/////////////////////////////////////////////////////////////////////
//          main data area ECC generator			   //
/////////////////////////////////////////////////////////////////////
void ECC_gen(unsigned char *Buffer,unsigned char *ECC_code){
 unsigned int i;
 unsigned char data,checksum,bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7;
 unsigned char LP0=0,LP1=0,LP2=0,LP3=0,LP4=0,LP5=0,LP6=0,LP7=0,LP8=0,LP9=0,LP10=0,LP11=0,LP12=0,LP13=0,LP14=0,LP15=0,LP16=0,LP17=0;
 unsigned char CP0=0,CP1=0,CP2=0,CP3=0,CP4=0,CP5=0;
 for(i=0;i<512;i++) {
   data=Buffer[i];
   bit0=data&(0x01);
   bit1=data&(0x02);
   bit2=data&(0x04);
   bit3=data&(0x08);
   bit4=data&(0x10);
   bit5=data&(0x20);
   bit6=data&(0x40);
   bit7=data&(0x80);
 
   checksum=data; //因為每個BIT再做XOR是多此一舉
   //checksum=bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0;
   
   if (i&0x01){LP1^=checksum;}else{LP0^=checksum;}
   if (i&0x02){LP3^=checksum;}else{LP2^=checksum;}
   if (i&0x04){LP5^=checksum;}else{LP4^=checksum;}
   if (i&0x08){LP7^=checksum;}else{LP6^=checksum;}
   if (i&0x10){LP9^=checksum;}else{LP8^=checksum;}
   if (i&0x20){LP11^=checksum;}else{LP10^=checksum;}
   if (i&0x40){LP13^=checksum;}else{LP12^=checksum;}
   if (i&0x80){LP15^=checksum;}else{LP14^=checksum;}
   if (i&0xA0){LP17^=checksum;}else{LP16^=checksum;}
   CP0^=(bit6^bit4^bit2^bit0);  //4次XOR
   CP1^=(bit7^bit5^bit3^bit1);  //4次XOR
   CP2^=(bit5^bit4^bit1^bit0);  //4次XOR
   CP3^=(bit7^bit6^bit3^bit2);  //4次XOR
   CP4^=(bit3^bit2^bit1^bit0);  //4次XOR
   CP5^=(bit7^bit6^bit5^bit4);  //4次XOR
 }
 ECC_code[6]=LP0|LP1|LP2|LP3|LP4|LP5|LP6|LP7;
 ECC_code[7]=LP8|LP9|LP10|LP11|LP12|LP13|LP14|LP15;
 ECC_code[8]=LP16|LP17|CP0|CP1|CP2|CP3|CP4|CP5;
}
 

/////////////////////////////////////////////////////////////////////
//     spare area ECC generator			 	           //
/////////////////////////////////////////////////////////////////////
void S_ECC_gen(unsigned char *Buffer, unsigned char *ECC_code){
 unsigned int i;
 unsigned char checksum,data,bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7;
 unsigned char S_LP0=0,S_LP1=0,S_LP2=0,S_LP3=0;
 unsigned char S_CP0=0,S_CP1=0,S_CP2=0,S_CP3=0,S_CP4=0,S_CP5=0;
 for(i=0;i<16;i++) {
   data=Buffer[i];
   bit0=data&(0x01);
   bit1=data&(0x02);
   bit2=data&(0x04);
   bit3=data&(0x08);
   bit4=data&(0x10);
   bit5=data&(0x20);
   bit6=data&(0x40);
   bit7=data&(0x80);
   checksum=bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0;
   if (i&0x01){S_LP1^=checksum;}else{S_LP0^=checksum;}
   if (i&0x02){S_LP3^=checksum;}else{S_LP2^=checksum;}
   S_CP0^=(bit6^bit4^bit2^bit0);
   S_CP1^=(bit7^bit5^bit3^bit1);
   S_CP2^=(bit5^bit4^bit1^bit0);
   S_CP3^=(bit7^bit6^bit3^bit2);
   S_CP4^=(bit3^bit2^bit1^bit0);
   S_CP5^=(bit7^bit6^bit5^bit4);	
 }
 ECC_code[9]=S_LP0|S_LP1|S_LP2|S_LP3|S_CP0|S_CP1|S_CP2|S_CP3;
 ECC_code[10]=S_CP4|S_CP5|0XFC;
}




