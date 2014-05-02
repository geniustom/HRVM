//#define OnlyTestAllCode  //�N��MARK����..�u�n���J����CODE..������k��s�B�B�@


/*
���{���Y�බ�Q�ϥ�..�̷ӥH�U�B�J�Ӵ���
1)RUN��CODE(ON RAM)
2)�H�N���J���N�{����TXT��
3)�t�XPC�ݵ{����s..��s������..�NDEVICE ��RESET
4)�Y�\�ೣ���`..�h��ܦ�FUNCTION OK
*/

/* �{���y�{: (memory map)
  |------------------|------0xFFFF
  | isr table        |------0xFFE0  if @0xFE00="0xFFFF">> NORMAL mode
  |------------------|------0xFE00  if @0xFE00="0xAAAA">> update mode
  | update isr table |
  |------------------|------0xFC00
  | code isr table   |  0xFA00-0xFBFF "ONLY" contented interrupt data
  |------------------|------0xFA00
  | update area      |
  |------------------|------0xF000
  |                  |
  | code area        |
  | [flash rom]      |
  |------------------|------0xFLASH ROM START
  | ram area         |
  |------------------|------0xRAM START
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


#include "Update/globol.h"        // Standard Equations
#include "../BootloaderExport.h"
#define CodeStart 0x2600
#define CodeEnd 0xFE00
//#define Flash_start_addr 0x2200
//__no_init unsigned int FlashAddr@0x2000; 
//__no_init unsigned int FlashAddr@Flash_start_addr;
#define RTCAddr1 0xEB00
unsigned int FlashAddr;

void main( void );
void ProcessUpdate(unsigned char RXBUF);
void flash_write_word (unsigned int value,unsigned int address) ;
void flash_write_Block (unsigned char *FBuf,unsigned int address) ;
void flash_erase_multi_segments (unsigned int Seg_Address,unsigned int Number_of_segments);
unsigned int LeftShift(unsigned int value,unsigned char power);
void Init_UART() ;
void UART0_SetBaudrate(unsigned long Baudrate);
void CheckUpdatePacket(unsigned char RxData) ;
void SendText(unsigned char SBUF[],unsigned int length);
void copy_codeisr2rom (void);
void Clear_Watchdog(void);
__interrupt void usart0_rx (void);

//-------------------------------------------------Dynamic Function Point-------------------------
//static unsigned long Page_Upload;
//const unsigned int StartUpdatingMode @0xFE00=0x0000;   //�p�G�w�g�}�l��s�A�N���ܦ��ƭȡA�۷��O�n�骺JUMP�A�����s������(�̫�@��BLOCK)�A�|�^�_��0XFFFF
UpdatePointer UpdateP;  //For Modify block;
static unsigned char USB[512]={0};
u8 Progress=0;
//------------------------------------------------------------------------------------------------
extern void read_data(u32 Page_index,u8 *Databuff);
extern void  Init_OLED(void);
extern void ShowProgress(u8 progress);
extern void ClearScreen();	//clear data buffer
extern void En_OLED();
/*
void Reset(){
  //WDTCTL = WDT_ARST_16;  //�}��WATCH DOG..�ó]��ACLKĲ�o�M��..16ms�᭫��
  //while(1);
  asm("MOV 0xFFFE,R14");
  asm("MOV @R14,PC");
}
*/
void ReallyUpgrate(){
  for(u32 i=CodeStart;i<=CodeEnd;i+=512){
    P6OUT ^= BIT5;
    read_data((u32)(FlashAddr+19+((i-CodeStart)>>9)),UpdateP.BUF);
    flash_erase_multi_segments(i,0); 
    __delay_cycles(100000);  //�쥻��
    flash_write_Block(UpdateP.BUF,i);
    __delay_cycles(10000);
    ShowProgress(Progress);
    Progress=128*(i-CodeStart)/(CodeEnd-CodeStart);
  }
  SendText("UPOK",4);
  _DINT();
  ShowProgress(128);
  //Reset();
}



u16 GetFlashAddr(){
   read_data(768,UpdateP.BUF);
   return UpdateP.BUF[0]*128+128;
}

void main( void ){
    WDTCTL = WDTPW + WDTHOLD;                // Stop WDT
    BCSCTL1=0x00;                             //&= ~XT2OFF; // XT2= HF XTAL
    do{
      IFG1 &= ~OFIFG;                         // Clear OSCFault flag
      for (int i = 0xFF; i > 0; i--);         // Time for flag to set
    }while ((IFG1 & OFIFG));                  // OSCFault flag still set?
    BCSCTL2 |= SELM_2+SELS;                   // MCLK=SMCLK=XT2 (safe)
    
    //Page_Upload=128;
    UpdateP.BUF=USB;
    P6DIR|=BIT5; //debug use
    //P6OUT|=BIT5; //debug use
    //NAND_ReSet();
    Init_OLED();
    En_OLED();
    ClearScreen();
    Init_UART();
    _DINT();
    FlashAddr=GetFlashAddr();
    ReallyUpgrate();
//---------------------------------------�Y0xFE00=AAAA��ܶi�J����s�Ҧ�--�o�ɤ~�n�}��UART    
    //_DINT();
    //_EINT();

    //_BIS_SR(LPM0_bits + GIE);             // Enter LPM0 w/interrupt
    while(1);
  //------------------------------------------------Boot Loader-------------------------
}

void  flash_write_word (unsigned int value,unsigned int address){
  unsigned int *Flash_ptr= (unsigned int *) address;    // Initialize Flash pointer
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  *Flash_ptr = value;                       // Write value to flash

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void  flash_write_Block (unsigned char *FBuf,unsigned int address){
  unsigned int *Flash_ptr= (unsigned int *) address;    // Initialize Flash pointer
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  for (int i=0;i<512;i+=2){
    if((FBuf[i]!=0xff)||(FBuf[i+1]!=0xff)){  //�p�G�O0xFF�N����s
      //*Flash_ptr =((unsigned int)FBuf[i]<<8)|(FBuf[i+1]);  //LEADTEK format // Write value to flash
      *Flash_ptr =((unsigned int)(FBuf[i])|(FBuf[i+1]<<8));   //Use TI Format
    }
    Flash_ptr++;
    __delay_cycles(1000);
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void flash_erase_multi_segments (unsigned int Seg_Address,unsigned int Number_of_segments)
{
  char *Flash_Ptr;                      // Flash pointer
  unsigned int i;
  for (i=0;i<=Number_of_segments;i++){
  Flash_Ptr = (char *) (Seg_Address + 512 * i) ;     // Initialize Flash pointer
  FCTL3 = FWKEY;                        // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                // Set Erase bit
  *Flash_Ptr = 0;                       // Dummy write to erase Flash segment
  FCTL1 = FWKEY;                        // Clear Erase bit
  FCTL3 = FWKEY + LOCK;                 // Reset LOCK bit
  }
}

void copy_codeisr2rom (void)
{
  unsigned char *Flash_ptrUpdate;               // Update area pointer
  unsigned char *Rom_ptr;                       // RAM pointer
  unsigned int i;
  //ISR table copy
  Flash_ptrUpdate = (unsigned char *) 0xFA00;   // Initialize main code isr pointer
  Rom_ptr = (unsigned char *) 0xFE00;   // Initialize ISR_rom pointer
  FCTL3 = FWKEY;                  // Clear Lock bit
  FCTL1 = FWKEY + WRT;            // Set WRT bit for write operation
  for (i=0; i<512; i++)  //512 Byte
  {
    *Rom_ptr++ = *Flash_ptrUpdate++; // copy value FLASH ROM to RAM
    __delay_cycles(1000);
  }
  FCTL1 = FWKEY;                  // Clear WRT bit
  FCTL3 = FWKEY + LOCK;           // Reset LOCK bit

}


void UART0_SetBaudrate(unsigned long Baudrate){
  switch (Baudrate){
      case 300: UBR00=0x92; UBR10=0x68; UMCTL0=0x4D; break;
      case 2400: UBR00=0x12; UBR10=0x0D; UMCTL0=0x9E; break;
      case 4800: UBR00=0x88; UBR10=0x06; UMCTL0=0x4B; break;
      case 9600: UBR00=0x44; UBR10=0x03; UMCTL0=0x9A; break;
      case 12800: UBR00=0x73; UBR10=0x02; UMCTL0=0x09; break;
      case 19200: UBR00=0xA1; UBR10=0x01; UMCTL0=0x49; break;
      case 38400: UBR00=0xD0; UBR10=0x00; UMCTL0=0x97; break;
      case 57600: UBR00=0x8A; UBR10=0x00; UMCTL0=0x0F; break;
      case 64000: UBR00=0x7D; UBR10=0x00; UMCTL0=0x05; break;
      case 76800: UBR00=0x68; UBR10=0x00; UMCTL0=0x1C; break;
      case 102400: UBR00=0x4E; UBR10=0x00; UMCTL0=0x7B; break;
      case 115200: UBR00=0x45; UBR10=0x00; UMCTL0=0xAA; break;
      case 230400: UBR00=0x22; UBR10=0x00; UMCTL0=0x99; break;
  }
  UCTL0 &= ~SWRST;
}


void SendText(unsigned char *SBUF,unsigned int length){
  for (int i=0;i<length;i++){
    while (!(IFG1 & UTXIFG0));                                   // USART0 TX buffer ready?
    TXBUF0 = SBUF[i];
  }
}

void Init_UART()
{
  P3SEL |= BIT4 | BIT5;                            // P3->4,5 = USART0 TXD/RXD
  ME1 |= UTXE0 + URXE0;                     // Enable USART0 TXD/RXD
  UCTL0 |= CHAR;                            // 8-bit character
  UTCTL0 |= SSEL1;                          // UCLK = SMCLK

  UART0_SetBaudrate(57600);

  //IE1 |= URXIE0;	    	// enable transmission and receiver intrrupt
}

/*
#pragma vector=UART0RX_VECTOR
__interrupt void usart0_rx (void){
 //while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
 //TXBUF0 = RXBUF0;                          // RXBUF0 to TXBUF0
  //unsigned char RX;
  //RX=RXBUF0;
  //ProcessUpdate(RX);
}
*/

/*
void Clear_Watchdog(void){
  WDTCTL = WDTPW + WDTCNTCL; //�M��WATCH DOG
}
*/
/*

void ProcessUpdate(unsigned char RXBUF){
  FCTL2 = FWKEY + FSSEL_2 + FN4 + FN2;      // SMCLK/20+1 = 400K for Flash Timing Generator
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
  else if(RXBUF=='q'||RXBUF=='Q'){ //-----------------------------���}UPDATE�Ҧ��A�í���
    //copy Code ISR table to "real" ISR table:0xFE00-0xFFFF
    //move point to 0xFFFE for reset device.
    //_DINT();
    //flash_erase_multi_segments(0xFE00,0);//erase isr table
    //__delay_cycles(1000000);
    //copy_codeisr2rom();
    //generate a reset
    flash_write_word(0x0000,0xFE00); //�i�J�u������s�Ҧ�
    Reset();
  }else{  //��L��������
    Donothing=1;
  }


  if(Donothing==0){ //�p�G���n����
     //===========================================================================
    if (UpdateP.Address_mode==1){  //��J����}
      UpdateP.Address_counter--;
      //Address|=(((unsigned int)RCVData)<<(4*Address_counter));  //����12,8,4,0
      UpdateP.Address|=LeftShift(RCVData,4*UpdateP.Address_counter);
      if(UpdateP.Address_counter==0){  //�뺡4BYTE->->���}ADDRESS MODE
        UpdateP.Address_mode=0;
        UpdateP.Data_counter =4;
        UpdateP.InputCount=0; //�M�Ŧ�0->->��512BYTE�~��FLASH�@�s��
        //printf("Address 0x%X ",UpdateP.Address);
        SendText((unsigned char *)AddressOK,9);
      }
    //===========================================================================
    }else{                 //��J��DATA��
      if (UpdateP.Data_counter>1){ //�뺡4BYTE->->�ন���
        UpdateP.Data_counter--;
        //Data|=(((unsigned int)RCVData)<<(4*Data_counter));  //����12,8,4,0
        UpdateP.Data|=LeftShift(RCVData,4*UpdateP.Data_counter);
      }else{               //�N��Ƽg�J�A�ê�l���ܼơA�����U4BYTE
        UpdateP.Data|=RCVData;
        UpdateP.BUF[UpdateP.InputCount]=(u8)((u16)UpdateP.Data>>8);
        UpdateP.BUF[UpdateP.InputCount+1]=(u8)(UpdateP.Data&0xff);
        //flash_write_word ((Data<<8)|(Data>>8),Address);
        UpdateP.Address+=2;
        UpdateP.InputCount+=2;
        UpdateP.Data_counter=4;
        //if (((Address-512)>=CodeBlockStart)&&(Address<CodeBlockEnd)&&(InputCount==512)){ //�Y�bCODE�ϰ�A�~��s
        //if (((UpdateP.Address<UpdateBlockStart)||(UpdateP.Address-512>UpdateBlockEnd))&&(UpdateP.InputCount==512)){ //�Y�bCODE�ϰ�A�~��s
        if (UpdateP.InputCount==512){ //�Y�bCODE�ϰ�A�~��s
           P6OUT ^= BIT5;    // Toggle P6.5 using exclusive-OR debug use
           //flash_erase_multi_segments(UpdateP.Address-512,0);   //�쥻��
           //__delay_cycles(1000000);  //�쥻��
           //flash_write_Block(UpdateP.BUF,UpdateP.Address-512);  //�쥻��
          //------------------------------------�~��FLASH����------------------------------------
           if (Page_Upload%32==0){  
             Block_Erase(Page_Upload);
           }
           save_data(Page_Upload,UpdateP.BUF); 
           Page_Upload++;  
          //------------------------------------�~��FLASH����------------------------------------
           P6OUT ^= BIT5;    // Toggle P6.5 using exclusive-OR debug use

           if (UpdateP.InputCount==512){
             SendText((unsigned char *)DataOK,6);
             if(UpdateP.Address-512==0xFE00)Reset();
           }
        }
        UpdateP.Data=0; //�n�k0->->�_�h�U����Ʒ|��
      }
    }
  }
}

unsigned int LeftShift(unsigned int value,unsigned char power){
  for(int i=0;i<power;i++){value<<=1;}
  return value;
}
*/
