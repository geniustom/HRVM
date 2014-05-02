
#include "RamUpdateSubProgram.h"
#include "../BootloaderExport.h"
#include "../../Driverlib/DriverExport.h"

//#define CSTART 0xE006
//#define CodeBlockStart 0x2600
//#define CodeBlockEnd 0xFE00
//#define UpdateBlockStart 0xE000
//#define UpdateBlockEnd 0xFDFF
//#define UpdateBlockEnd 0xFEDF 


/*
�`�N�ƶ��G
1.�ϥ�MAKEFILE..�NRamUpdateSubProgram.C �ন .h
2.�NCODE�Ϫ��_�l��}�אּ0xF000 (update area start)
3.�NISR�אּISRA..ISRB..��}�n�令FDXX
  �ҡG0xFFF2,       // Start address of section 
      �אּ const WORD ISRA[] @0xFDF2 ={0x1726};

*/
/* �{���y�{: (memory map) for MSP430F1612
  |------------------|------0xFFFF  \
  | isr table        |---0xFFE0     / ISR Area
  |                  |
  |------------------|------0xFE00  RealIsrPointer
  | update isr table |---0xFDE0  update isr start
  |                  |
  |------------------|------0xFC00
  | code isr table   |---0xFA00-0xFBFF "ONLY" contented interrupt data
  |                  |
  |------------------|------0xFA00
  | update area      |
  |------------------|------0xED00
  |                  |
  | code area        |
  | [flash rom]      |
  |------------------|------0x2600 FLASH ROM START
  | ram area         |
  |                  | =RamCodePointer 0x131E 
  |------------------|------0x1100 START
  | SFR area         |
  |------------------|------0x0000
�}�����˴��O�_�n�i�Jupdate mode�A�p�G�O���ܡA�h
(0)�P�_0xFE00�B�O�_��0xFF�A�Y�_�h��update mode
(1)�N0xF000-0xF400 ��m����ƽƻs��RAM��
(2)�N0xFC00(update isr table)�ƻs��isr table��
(3)pc���V�{���ҩl��m(��reset)
(4)�{���M�� code area �ö}�l��/�g��Ʀ�code area
(5)��code isr table
(6)�N code isr table�Ƽg��isr table
(7)���} update mode�Apc���V�{���ҩl��m(��reset)
(8)����
*/

UpdatePointer UpdateP;  //For Modify block;
//------------------------------------------------ FIXFunctionPoint Block-------------------------
void RefreshFIXFunctionPoint();
//extern void Run_Main();
//extern void DoNormalRx(unsigned char RxData);
//extern void FSave_Data(unsigned long Page_index,unsigned char *Databuff);
//extern void Block_Erase(unsigned long Page_index);
//-------------------------------------------------------------Fix Block--------------------------
//void main( void );
void CheckUpdatePacket(unsigned char RxData);
void UART0RX(unsigned char RX);
void copy_update2ram (void);
//void Reset(void);
//void Clear_Watchdog(void);
//-------------------------------------------------Dynamic Function Point-------------------------
//#define Flash_start_addr 0x2200
__no_init unsigned char USB[512]@0x2100;
//__no_init unsigned int Flash_start_ptr@Flash_start_addr;
//#define RTCAddr1 0xEB00

const unsigned char AddressOK[] ="AddressOK";
const unsigned char DataOK[] ="DataOK";
const unsigned char Upload_STR[] ="Upload";
unsigned int UPP;
unsigned long Page_Upload;
unsigned char CHKSUM=0x00;
const unsigned int StartUpdatingMode @0xFE00=0xFFFF;   //�p�G�w�g�}�l��s�A�N���ܦ��ƭȡA�۷��O�n�骺JUMP�A�����s������(�̫�@��BLOCK)�A�|�^�_��0XFFFF
//const unsigned int StartUpdatingMode @0xFE00=0xAAAA;   //���է�s�\��O�_���`�Ϊ�
//------------------------------------------------------------------------------------------------
//extern void NAND_ReSet();


void StartDownload(){ //��s�ǳƼҦ�
  //-----------------------CLOSE TIMER
    TACCR0 = 0;
    TACTL = 0;      // ACLK, clear TAR, up mode
    TACCTL0 = 0;
    TACCR0 = 0;
    TBCTL=0;
  //-----------------------  
    unsigned int sec;
    sec=CodeArray[0];//dummy for insure
    sec=ISR_A[0];
    sec=ISR_B[0];
    if (sec==0)_NOP();
  //-----------------------�H�W�O�קKCOMPILER�̨Τ�    
   _DINT();
   UpdateP.Updating=1;
   UpdateP.RestoreISR=0;
   flash_write_word(0xAAAA,0xFE00);
   WDTCTL = 0;  //RESET SYSTEM
}

void StartUpload(){ //��s�Ҧ�
 //copy Code ISR table to "real" ISR table:0xFE00-0xFFFF
 //move point to 0xFFFE for reset device.
  copy_update2ram();
  WDTCTL = 0;  //RESET SYSTEM
}


void WriteTheBlockCanUSE(unsigned int PageAddr){
   BlockErase(768,1);
   for (int i=0;i<512;i++){UpdateP.BUF[i]=0;}  //0~256
   UpdateP.BUF[0]=PageAddr;
   FSave_Data(768,UpdateP.BUF);
}

void ShowMSGtoOLED(unsigned char *SBUF,unsigned int length,unsigned char Abort){
  Init_OLED();
  ClearScreen();
  Print(SBUF,length,2,2,8);
  if(Abort==1)Print(" PRESS TO ABORT",15,2,52,8);
  show_pic();
}
/*
void DisableAllInterrupt(){
  IE1=0x00;
  IE2=0x00;
  //WDTIE=0x00;
  //OFIE=0x00;
  //NMIIE=0x00;
  //ACCVIE=0x00;
  I2CIE=0x00;
  //CAIE=0x00;
  ADC12IE=0x00;
  //ADC12TOVIE=0x00;
  //ADC12OVIE=0x00;
  //DAC12IE=0x00;
  //DMAIE=0x00;
  //URXEIE=0x00;
  //TAIE=0x00;
  //TBIE=0x00;
  //CCIE=0x00;
  P1IE=0x00;
  P2IE=0x00;
}
*/


void RestoreISR(){
    UpdateP.RestoreISR=0;
    _DINT();
    unsigned char *Flash_ptrUpdate=(unsigned char *) RealIsrPointer;               // Update area pointer
    for(int i=0;i<512;i++){
      USB[i]=*Flash_ptrUpdate;
      Flash_ptrUpdate++;
    }
    USB[0]=0xff; USB[1]=0xff;
 /////////////////////////////////////////////////�NISR�٭�///////////////////////
    Flash_ptrUpdate=(unsigned char *) RealIsrPointer;
    FCTL1 = FWKEY + ERASE;          // Set Erase bit
    FCTL3 = FWKEY;                  // Clear Lock bit
    *Flash_ptrUpdate = 0;                 // Dummy write to erase ISR Area
    FCTL1 = FWKEY + WRT;            // Set WRT bit for write operation
    for (int i=0; i<512; i++){
      *Flash_ptrUpdate++=USB[i]; // copy value FLASH ROM to RAM
    }
    FCTL1 = FWKEY;                  // Clear WRT bit
    FCTL3 = FWKEY + LOCK;           // Reset LOCK bit    
/*////////////////////////////////////////////////�NStartUpdatingMode�٭�//////////////////////////
    Flash_ptrUpdate=(unsigned char *) StartUpdatingMode;
    FCTL1 = FWKEY + ERASE;          // Set Erase bit
    FCTL3 = FWKEY;                  // Clear Lock bit
    *Flash_ptrUpdate = 0;                 // Dummy write to erase ISR Area
    FCTL1 = FWKEY + WRT;            // Set WRT bit for write operation
    *Flash_ptrUpdate = (unsigned char)0xffff;
    FCTL1 = FWKEY;                  // Clear WRT bit
    FCTL3 = FWKEY + LOCK;           // Reset LOCK bit 
/////////////////////////////////////////////////RESET���l���A*/
    ClearScreen();
    Print("  PLEASE RESET ",15,2,52,8);
    show_pic();
    //Reset();
}


unsigned char SelfFunctionTest(){   //TEST FLASH ROM IS OK,AND TELL PC THE UPLOAD ADDRESS
  unsigned char CanUsePage=0;
  unsigned char ERR=0;
  const unsigned int CheckPage[5]={128,256,384,512,640};
  unsigned char SelfTest_STR[6] ={'F','l','a','s','h','0'};
  ShowMSGtoOLED("FLASH TEST",13,0);
  //NAND_ReSet();
  for (int j=0;j<5;j++){
    ERR=0;
    for(int k=0;k<128;k++){
      if((CheckPage[j]+k)%32==0){BlockErase(CheckPage[j]+k,1);}
      for (int i=0;i<512;i++){UpdateP.BUF[i]=i;}  //0~256
      FSave_Data(CheckPage[j]+k,UpdateP.BUF);
      for (int i=0;i<512;i++){UpdateP.BUF[i]=0;} //�k�s
      FRead_Data(CheckPage[j]+k,UpdateP.BUF);
      for (int i=0;i<512;i++){if(UpdateP.BUF[i]!=(unsigned char)(i)){ERR=1;break;}} //�k�s
      if (ERR==1){CanUsePage++;break;}
    }
    if(ERR==0)break;  //��ܳo��PAGE��Ū�g���S���D..�i�H�}�l�g�J
  }
  SelfTest_STR[5]=CanUsePage;
  UpdateP.FwStartPage=CheckPage[CanUsePage];
  WriteTheBlockCanUSE(CanUsePage);
  ShowMSGtoOLED("UP FW WAITING",13,1);
  OpenUart(0,57600,&UART0RX);
  _EINT();
  SendTextToUart(0,(unsigned char *)SelfTest_STR,6); //�e��PC�ݦۧ��˴�����..�i�H�}�l�Ǹ�ƨ�
  return CanUsePage;
}


void ComputeProgress(){
  unsigned char PSTR[13]={'F','W','2','.','2',' ','L','O','A','D',' ','0','0'};
   UpdateP.Progress=(unsigned int)((unsigned long)(UpdateP.Address)*100/65536);
   PSTR[11]=0x30+(UpdateP.Progress/10);
   PSTR[12]=0x30+(UpdateP.Progress%10);
   ClearScreen();
   Print(PSTR,13,2,2,8);
   Print(" PRESS TO ABORT",15,2,52,8);
   show_pic();
}

/*
void Reset(){
   //IE1=0;
   //_DINT();
   //WDTCTL = WDT_ARST_16;  //�}��WATCH DOG..�ó]��ACLKĲ�o�M��..�@��᭫��
   //while(1);
  //WDTCTL = WDT_ADLY_16;               // WDT 1000ms, ACLK, interval timer
  //IE1 |= WDTIE;                         // Enable WDT interrupt
  asm("MOV 0xFFFE,UPP");
  asm("MOV UPP,PC");
}
*/

void Clear_Watchdog(void){
  WDTCTL = WDTPW + WDTCNTCL; //�M��WATCH DOG
}



void RefreshFIXFunctionPoint(){ //�NMain�|�Ψ쪺�DFIX��FUNCTION POINT�b������l��->->�H�קK��s���Ʀ�}���ܤ����D
  ///UPP=(UpdatePointer *)UpdatingPointer;  //UPP�Y�OUpdateP->->UPP�PUpdateP�@�ΦP�@��}->->��UpdateP���ܦ�m��->->FIX BLOCK���i�o�����T��m
  //UpdateP.Run_MainFunct=Run_Main;         //���קK��s����䤣��->->�G�����@�ʺA����
  //UpdateP.DoNormalRxFunct=DoNormalRx;   //���קK��s����䤣��->->�G�����@�ʺA����
  UpdateP.CheckUpdateIndex=0;
  UpdateP.Address_counter=0;
  UpdateP.Address=0x0000;
  UpdateP.Data_counter=0;
  UpdateP.InputCount=0;
}


void CheckUpdatePacket(unsigned char RxData){
  if (RxData=='U'){           //����Ĥ@�Ӧr�OU�N�}�l�έp
    UpdateP.CheckUpdateIndex=0;
    for(int i=0;i<=7;i++)UpdateP.UpdatingPacket[i]=0x00;
  } 
  //�}�l�έp��->->�̧ǥ�JDATA
  UpdateP.UpdatingPacket[UpdateP.CheckUpdateIndex]=RxData;
  UpdateP.CheckUpdateIndex++;

  if(UpdateP.CheckUpdateIndex==6){    //�P�_���쪺�r�O�_�O'Upload'
    char Same=1;
    for(int i=0;i<=5;i++){
      if (Upload_STR[i]!=UpdateP.UpdatingPacket[i])Same=0;
    }
    UpdateP.CheckUpdateIndex=0;
    for(int i=0;i<=7;i++)UpdateP.UpdatingPacket[i]=0x00;
    if(Same==1){
      StartDownload();
    }  //�ۦP���ܡC�i�J��s�Ҧ�
  }

}


void copy_update2ram (void){
  unsigned char *Flash_ptrUpdate;               // Update area pointer
  unsigned char *Ram_ptr;                       // RAM pointer
  unsigned int i;
//Flash to RAM copy---------------------------------------------------------------------------------------------
  Flash_ptrUpdate = (unsigned char *) UpdateCodePointer;   // Initialize Flash ROM pointer
  Ram_ptr = (unsigned char *) RamCodePointer;   // Initialize RAM pointer
  for (i=0; i<4096; i++){  //4096 Byte
      *Ram_ptr++ = *Flash_ptrUpdate++; // copy value FLASH ROM to RAM
  }
//ISR Table copy---------------------------------------------------------------------------------------------
  Flash_ptrUpdate = (unsigned char *) UpdateIsrPointer;   // Initialize Flash segment A pointer
  Ram_ptr = (unsigned char *) RealIsrPointer;           // Initialize Flash segment B pointer
  FCTL1 = FWKEY + ERASE;          // Set Erase bit
  FCTL3 = FWKEY;                  // Clear Lock bit
  *Ram_ptr = 0;                 // Dummy write to erase ISR Area
  FCTL1 = FWKEY + WRT;            // Set WRT bit for write operation
  for (i=0; i<512; i++)  //512 Byte
  {
    *Ram_ptr++ = *Flash_ptrUpdate++; // copy value FLASH ROM to RAM
  }
  FCTL1 = FWKEY;                  // Clear WRT bit
  FCTL3 = FWKEY + LOCK;           // Reset LOCK bit
  //--------------------------------------------COPY�i�Ϊ�FLASH ROM �Ŷ�
  
   //�q�Ȧs�����s�^��
}

unsigned int LeftShift(unsigned int value,unsigned char power){
  for(int i=0;i<power;i++){value<<=1;}
  return value;
}


void ProcessUpdate(unsigned char RXBUF){
  //FCTL2 = FWKEY + FSSEL_2 + FN4 + FN2;      // SMCLK/20+1 = 400K for Flash Timing Generator
  _NOP();
  unsigned char RCVData;
  unsigned char Donothing=0;
  if(RXBUF>=0x30&&RXBUF<=0x39){ //--------------------------------0-9 ->0~9
    RCVData=RXBUF-0x30;
  }
  else if(RXBUF>=0x41&&RXBUF<=0x46){ //---------------------------A-F ->10~15
    RCVData=RXBUF-0x41+10;
  }
  else if(RXBUF==0x40){ //-----------------------------------------@ ����᪺��4��BYTE������}
    UpdateP.Address_mode=1;
    UpdateP.Address=0;
    UpdateP.Data=0;
    UpdateP.Address_counter=5;
  }
  else if(RXBUF=='r'||RXBUF=='R'){ //-----------------------------���}�ثe���ǿ�A�^��쥻�{��(�קK������)
    RestoreISR();
  }
  else if(RXBUF=='q'||RXBUF=='Q'){ //-----------------------------���}UPDATE�Ҧ��A�í���
    Dis_OLED();
    Reset();
  }else{  //�����L�r��..��������
    Donothing=1;
  }


  if(Donothing==0){ //�p�G���n����
     //===========================================================================
    if (UpdateP.Address_mode==1){  //��J����}
      UpdateP.Address_counter--;
      //Address|=(((unsigned int)RCVData)<<(4*Address_counter));  //����12,8,4,0
      UpdateP.Address|=LeftShift(RCVData,4*UpdateP.Address_counter);
      Page_Upload=(unsigned long)((unsigned long)UpdateP.Address/512)+UpdateP.FwStartPage;
      if(UpdateP.Address_counter==0){  //�뺡4BYTE->->���}ADDRESS MODE
        UpdateP.Address_mode=0;
        UpdateP.Data_counter =4;
        UpdateP.InputCount=0; //�M�Ŧ�0->->��512BYTE�~��FLASH�@�s��
        SendTextToUart(0,(unsigned char *)AddressOK,9);
        //printf("A: 0x%X ",UpdateP.Address);
      }
    //===========================================================================
    }else{                 //��J��DATA��
      if (UpdateP.Data_counter>1){ //�뺡4BYTE->->�ন���
        UpdateP.Data_counter--;
        //Data|=(((unsigned int)RCVData)<<(4*Data_counter));  //����12,8,4,0
        UpdateP.Data|=LeftShift(RCVData,4*UpdateP.Data_counter);
      }else{               //�N��Ƽg�J�A�ê�l���ܼơA�����U4BYTE
        UpdateP.Data|=RCVData;
        UpdateP.BUF[UpdateP.InputCount]=(unsigned char)((unsigned int)UpdateP.Data>>8);
        UpdateP.BUF[UpdateP.InputCount+1]=(unsigned char)(UpdateP.Data&0xff);
        //flash_write_word ((Data<<8)|(Data>>8),Address);
        UpdateP.Address+=2;
        UpdateP.InputCount+=2;
        UpdateP.Data_counter=4;
        //if (((Address-512)>=CodeBlockStart)&&(Address<CodeBlockEnd)&&(InputCount==512)){ //�Y�bCODE�ϰ�A�~��s
        //if (((UpdateP.Address<UpdateBlockStart)||(UpdateP.Address-512>UpdateBlockEnd))&&(UpdateP.InputCount==512)){ //�Y�bCODE�ϰ�A�~��s

        if (UpdateP.InputCount==512){ //�Y�bCODE�ϰ�A�~��s
           P6OUT ^= BIT5;    // Toggle P6.5 using exclusive-OR debug use
           ComputeProgress();
          //------------------------------------�~��FLASH����------------------------------------
           if (Page_Upload%32==0){  
             BlockErase(Page_Upload,1);
           }
           FSave_Data(Page_Upload,UpdateP.BUF); 
           //-----------------------------------COMPUTE CHECK SUM-------------------------
           _DINT();
           CHKSUM=0x00;
           FRead_Data(Page_Upload,UpdateP.BUF);
           for(int i=0;i<512;i++){CHKSUM=CHKSUM^UpdateP.BUF[i];}
           //if (CHKSUM==17)CHKSUM=71; //�����D������..�qMCU�e17��PC���|����0..�G��71���N
           unsigned char BUFSTR[9];
           for(int CC=0;CC<6;CC++){BUFSTR[CC]=DataOK[CC];}
           BUFSTR[6]=0x30+CHKSUM/100;
           BUFSTR[7]=0x30+(CHKSUM/10)%10;
           BUFSTR[8]=0x30+CHKSUM%10;
           _EINT();
           SendTextToUart(0,(unsigned char *)BUFSTR,9);
           //-----------------------------------COMPUTE CHECK SUM-------------------------
           if(UpdateP.Address-512==0xFE00){  //�̫�@�ӧ�s������BLOCK
             _DINT();
             flash_write_word(0x0000,0xFE00); //�i�J�u������s�Ҧ�
             Reset();
           }

         }
        UpdateP.Data=0; //�n�k0->->�_�h�U����Ʒ|��
      }
    }
  }
}


void UART0RX(unsigned char RX){
  if(UpdateP.Updating==0){
    UpdateP.DoNormalRxFunct(RX);
    CheckUpdatePacket(RX); //check the Update keyword,if detect then copy flash to ram and reset
  }else{
    ProcessUpdate(RX);
  }
}

void BootLoaderMain(void(*Run_MainFunct)(void),void(*DoNormalRxFunct)(unsigned char RxData)){ //�ǤJUART ISR�P�l�{��MAIN����ƫ���
    WDTCTL = WDTPW | WDTHOLD;                // Stop WDT
    
    Use_XTAL2();
    
    UpdateP.BufFulltoWrite=0;
    switch(StartUpdatingMode){
      case 0xFFFF: UpdateP.Updating=0; break;  //�@��Ҧ�
      case 0xAAAA: UpdateP.Updating=1; break;  //��s�ǳƼҦ�
      case 0x0000: UpdateP.Updating=2; StartUpload(); break;  //��s�Ҧ�
    }
    RefreshFIXFunctionPoint();
 //----------------------------------------------INIT---------------------------------------
    _DINT();
    OpenUart(0,57600,&UART0RX);
    Init_Flash();
    _EINT();  
    if(UpdateP.Updating==1){ //�p�G����FFFF..��ܤW�����}�e�w�i�J��s�Ҧ�..�o��..�N���s�{��COPY��RAM���í���  
      ShowMSGtoOLED("FW UPGRATE2.3",13,1);
      UpdateP.BUF=USB;
      Page_Upload=UpdateP.FwStartPage;
      P6DIR|=BIT5; //debug use
      P6OUT|=BIT5; //debug use
      UpdateP.Progress=0;
      _DINT();
      //DisableAllInterrupt();
      SelfFunctionTest();
    };  
    UpdateP.Run_MainFunct=Run_MainFunct;         //���קK��s����䤣��->->�G�����@�ʺA����
    UpdateP.DoNormalRxFunct=DoNormalRxFunct;   //���קK��s����䤣��->->�G�����@�ʺA����
//----------------------------------------------INIT---------------------------------------
    while(UpdateP.Updating!=0){
      P2DIR&=~BIT0; //�]�w����J
      if (((P2IN&BIT0)!=BIT0)&&(UpdateP.RestoreISR==0)){RestoreISR();}
    }; //�Y���s�Ҧ�..�N��w�b�o..������Run_Main
//------------------------------------------------Boot Loader-------------------------------------------------------------------------
    UpdateP.Run_MainFunct();
}


