#include "Nand.h"
#include "../DriverExport.h"

void Init_Flash(void){
  FLASHBus = 0;
  FLASHBus |= CLE;    //�]�mCLE����X�޸} P3.0
  FLASHBus |= CE;    //�]�mCE~����X�޸} P3.2
  FLASHBus |= ALE;    //�]�mALE����X�޸} P3.1
  FLASHBus |= RE;    //�]�mRE~����X�޸} P3.3
  FLASHBus |= WE;    //�]�mWE~����X�޸} P2.4
  FLASHBus |= WP;    //�]�mWP����X�޸} P2.5
  FLASHControl |= WP;    
  RBPort &= ~RB; //�]�mR/B����J�޸} P2.7
}

void Reset_Flash(void){ 
  FLASHDIR = 0xff;			// �]�mP1�f����X��V
  CE_Enable;
    CLE_Enable;
      WE_Enable;
        FLASHDATAOUT = 0xff;			// ��XRESET����R�O
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
      FLASHDATAOUT = (unsigned char)(nCol);//�e���}
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD1);//�e�C��}BYTE1
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD2);//�e�C��}BYTE2
    WE_Disable;
    WE_Enable;
      FLASHDATAOUT = (unsigned char)(nADD3);//�e�C��}BYTE3
    WE_Disable;
  ALE_Disable;
}

void SendFlashCMD(unsigned char CMD){
  CLE_Enable;
    WE_Enable;
      FLASHDATAOUT = CMD;			//��X�����X�R�O
    WE_Disable;
  CLE_Disable;

}
////////////////////////////////////////
// ���T��^ 1�A���~��^ 0
int BlockErase(unsigned long nAddr,unsigned char BusyWait){
  int nTemp = 0;
  FLASHDIR = 0xff;			//�]�mP1�f����X��V
  
  CE_Enable;   
    SendFlashCMD(0x60);
    EraseAddress(nAddr); 
    SendFlashCMD(0xd0);
    for(int i = 50;i > 0;i--);
    FLASHDIR = 0x00;				//�]�mP1�f����J��V
    RE_Enable;
      nTemp = FLASHDATAIN;
    RE_Disable; 
  CE_Disable;
  
  if(BusyWait==1)while(RBIO&RB==0)_NOP();
  //for(int i=0;i<5000;i++);  //��������nDELAY5000..�_�h�g�|�����D
  if(nTemp & 0x01) return 0;
  else return 1;
}


void PageRead(unsigned int nCol,unsigned long nRow,unsigned char *pBuf,unsigned int Len){
  FLASHDIR = 0xff;		//�]�mP1�f����X��V
  unsigned char fCMD=(unsigned char)(nCol>>8);
  if(nCol>=512){//�s���Q�ΰ�
    fCMD=0x50;
    nCol-=512;
  }
  
  CE_Enable;  
    SendFlashCMD(fCMD); 
    AccessAddress(nCol,nRow);
    FLASHDIR = 0x00;			//�]�mP1�f����J��V
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
// ���T��^ 1�A���~��^ 0
int PageWrite(unsigned int nCol,unsigned long nRow,unsigned char *pBuf,unsigned int Len){
  int nTemp = 0;
  unsigned char fCMD=(unsigned char)(nCol>>8);
  if(nCol>=512){//�s���Q�ΰ�
    fCMD=0x50;
    nCol-=512;
  }
  FLASHDIR = 0xff;			//�]�mP1�f����X��V
  
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
    FLASHDIR = 0x00;			//�]�mP1�f����J��V

    for(unsigned int i= 5000;i > 0;i--){             //�p�G��READY�N���򵥫�
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
  S_ECC_gen(Sparebuff,Sparebuff1); 		   		//����S_ECC code��ECC1
  Sparebuff[9]=data_buff1;
  Sparebuff[10]=data_buff2;
  
  PageRead(0,Page_index,Databuff,PAGE_DATA_SIZE);               //Ū�����        
  ECC_gen(Databuff,Sparebuff1); 		   		//���ͷs�XECC code��ECC1
  PageRead(512,Page_index,Sparebuff,PAGE_SPARE_SIZE);           //Ū����ECC code��ECC
}

/*
int WriteByte(unsigned int nCol,unsigned long nRow,unsigned char nValue){
  int nTemp = 0;
  //�]��FLASH�����}�O�@��BYTE..�μg�J���ΰϨӨM�w�_��BYTE�ΰ���BYTE
  unsigned char fCMD=(unsigned char)(nCol>>8);
  if(nCol>=512){//�s���Q�ΰ�
    fCMD=0x50;
    nCol-=512;
  }
  FLASHDIR = 0xff;			//�]�mP1�f����X��V 

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
    FLASHDIR = 0x00;			//�]�mP1�f����J��V
    for(unsigned int i = 50000;i > 0;i--){  //�p�G��READY�N���򵥫�
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
  //�]��FLASH�����}�O�@��BYTE..�μg�J���ΰϨӨM�w�_��BYTE�ΰ���BYTE
  if(nCol>=512){//�s���Q�ΰ�
    fCMD=0x50;
    nCol-=512;
  }
  FLASHDIR = 0xff;		//�]�mP1�f����X��V
  
  CE_Enable;
    SendFlashCMD(fCMD);
    AccessAddress(nCol,nRow);
    FLASHDIR = 0x00;		//�]�mP1�f����J��V
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
  j=i1|i2; //j=0 ��ܵLERROR
*/  
/*
  int L=0,l1=0,l2=0,l3=0;
  l1=Sparebuff[6]^Sparebuff1[6]; 	   			//XOR the ECC code
  l2=Sparebuff[7]^Sparebuff1[7];
  l3=Sparebuff[8]^Sparebuff1[8];
  L=l1|l2|l3;  //L=0 ��ܵLERROR
*/

