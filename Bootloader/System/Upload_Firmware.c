#include "msp430x16x.h"        // Standard Equations
#include "System/IO/PortManager.h"
#include "System/Upload_Firmware.h"
#define CSTART 0xE006
#define CodeBlockStart 0x2600
#define CodeBlockEnd 0xDFFF
#define UpdateBlockStart 0xE000
#define UpdateBlockEnd 0xFDFF
//#define UpdateBlockEnd 0xFEDF 
/*
         |-------------------------|           
         |  Code Block(can modify) |  0x2500~0xE000(所有的CODE都算->->故不用特別更改)
         |-------------------------| 
         |   UpdateBlock(readonly) |  0xE000~0xFEDF(負責更新的PROGRAM)
         |-------------------------| 
*/
//const unsigned int UpdatingPointer @0x2700=(unsigned int)&Updating;  <--宣告靜態指標變數的方法
//const unsigned int UpdatingPointer @0x2700= (unsigned int)&CloseAllInterrupt; <--宣告靜態函數指標的方法 ( CloseAllInterrupt(void) )
//const unsigned int UpdatingPointer[] @0x2700={((unsigned int)&Updating),((unsigned int)&USBInStart)}; <--宣告靜態指標陣列的方法
//void (*UpdatingPointer)(void) = &CloseAllInterrupt;  <--直接宣告動態函數指標的方法
//unsigned char UP;
//const unsigned int UpdatingPointer @0x2700=(unsigned int)&Updating;
//const unsigned int UpdatingPointer @0x2700=(unsigned int)&UpdateP; 

//------------------------------------------------ FIXFunctionPoint Block-------------------------
void RefreshFIXFunctionPoint() @ "FIXFP";
extern void Run_Main();
extern void DoNormalRx(u8 RxData);
//-------------------------------------------------------------Fix Block--------------------------
void main( void ) @"FIX";
void ProcessUpdate(unsigned char RXBUF) @ "FIX";
void flash_write_word (unsigned int value,unsigned int address) @ "FIX";
void  flash_write_Block (unsigned char *FBuf,unsigned int address) @ "FIX";
void flash_erase_multi_segments (unsigned int Seg_Address,unsigned int Number_of_segments) @ "FIX";
unsigned int LeftShift(unsigned int value,unsigned char power) @ "FIX";
unsigned char Init_UART() @ "FIX";
void UART0_SetBaudrate(unsigned long Baudrate) @ "FIX";
void CheckUpdatePacket(unsigned char RxData) @ "FIX";
void SendText(unsigned char SBUF[],unsigned int length) @ "FIX";
__interrupt void usart0_rx (void) @ "FIX";
void Reset(void) @ "FIX";
void Clear_Watchdog(void) @ "FIX";

void UP_Init_OLED() @ "FIX";
void UP_Dis_OLED() @ "FIX";
void UP_En_OLED() @ "FIX";
void UP_show_pic() @ "FIX";
void UP_OLED_Reset() @ "FIX";
void UP_initial_ssd1303() @ "FIX";
void UP_set_col() @ "FIX";
//-------------------------------------------------Dynamic Function Point-------------------------
//__no_init unsigned char USB[512] @0x2300; //用0x1200沒法測ECG..自行規劃的RAM空間，要從0x2500往前算
unsigned char USB[512];
//const unsigned int USBPointer @0x2700=(unsigned int)&USB;
__no_init UpdatePointer UpdateP @0x2280;  //For Modify block;
//UpdatePointer UpdateP;
const unsigned char AddressOK[] @0xF800 ="AddressOK";
const unsigned char DataOK[] @0xF810 ="DataOK";
const unsigned char Upload_STR[] @0xF820 ="Upload";
const unsigned char UP_initial[18] @0xF830={ 0xE2,0xAE,0xAD,0x8A,0xA0,0xC8,0xA8,0x3F,0x81,0x80,0xD5,0x10,0xDB,0xFF,0xA4,0xAF};
const unsigned int StartUpdatingMode @0xFE00=0xFFFF;   //如果已經開始更新，就改變此數值，相當於是軟體的JUMP，直到更新完成後(最後一個BLOCK)，會回復成0XFFFF
//------------------------------------------------------------------------------------------------

void RefreshFIXFunctionPoint(){ //將Main會用到的非FIX區FUNCTION POINT在此先初始化->->以避免更新後函數位址改變之問題
  //UPP=(UpdatePointer *)UpdatingPointer;  //UPP即是UpdateP->->UPP與UpdateP共用同一位址->->當UpdateP改變位置時->->FIX BLOCK仍可得知正確位置
  UpdateP.Run_MainFunct=Run_Main;         //為避免更新之後找不到->->故給予一動態指標
  UpdateP.DoNormalRxFunct=DoNormalRx;   //為避免更新之後找不到->->故給予一動態指標
  UpdateP.BUF=USB;
  UpdateP.CheckUpdateIndex=0;
  UpdateP.Address_counter=0;
  UpdateP.Address=0x0000;
  UpdateP.Data_counter=0;
  UpdateP.InputCount=0;
}

void main( void )
{
    WDTCTL = WDTPW | WDTHOLD;                // Stop WDT
    BCSCTL1 &= ~XT2OFF; // XT2= HF XTAL &= ~XT2OFF;
    do{
      IFG1 &= ~OFIFG; // Clear OSCFault flag
      for (int i = 0xFF; i > 0; i--); // Time for flag to set
    }while ((IFG1 & OFIFG)); // OSCFault flag still set? 
    BCSCTL2 |= SELM_2+SELS; // MCLK=SMCLK=XT2 (safe)
    //FCTL2 = FWKEY + FSSEL_2 + FN4 + FN2;
    // SMCLK/20+1 = 400K for Flash Timing Generator
    //  16*FN4 + 8*FN3 + 4*FN2 + 2*FN1 + FN0 + 1
    
    //printf("R\n");
    //UpdateP.BUF=(u8 *)USBPointer;  //USBPointer即是USB->->UpdateP.BUF與USBPointer共用同一位址->->當USB改變位置時->->FUSBPointer仍可得知正確位置
    UpdateP.BUF=USB;
    UpdateP.BufFulltoWrite=0;
    UpdateP.Updating=(StartUpdatingMode==0xFFFF)? 0 : 1; //如果StartUpdatingMode<>0xff..表示UpdateP.Updating為1
    if(UpdateP.Updating!=0)UP_Init_OLED();
    _DINT();
    Init_UART();
    _EINT();
//----------------------------------------------INIT---------------------------------------
    //WDTCTL = WDT_ARST_1000;  //開啟WATCH DOG..並設成ACLK觸發清除
    while(UpdateP.Updating!=0){
      if (UpdateP.BufFulltoWrite!=0){
         if((UpdateP.Address-512)==0xFE00){_DINT();} //因為寫最後一個BLOCK時，要避免UART發生中斷，找不到中斷向量，故DISABLE INTERRUPT
         flash_erase_multi_segments(UpdateP.Address-512,0);
         flash_write_Block(UpdateP.BUF,UpdateP.Address-512);
         UpdateP.BufFulltoWrite=0;
         if((UpdateP.Address-512)==0xFE00){_EINT();} //寫入完畢，變成更新的中斷向量，ENABLE INTERRUPT
      }
      UP_show_pic();
      //Clear_Watchdog();
    }; //若不為0..就在此HOLD住了
    //WDTCTL = WDTPW + WDTHOLD;                // Stop WDT
    UP_Dis_OLED();
//------------------------------------------------Boot Loader-------------------------  
    RefreshFIXFunctionPoint();

    UpdateP.Run_MainFunct();
}

void Reset(void){
    asm("MOV #0x0000,R4");
    asm("MOV #0x0000,R5");
    asm("MOV #0x0000,R6");
    asm("MOV #0x0000,R7");
    asm("MOV #0x0000,R8");
    asm("MOV #0x0000,R9");
    asm("MOV #0x0000,R10");
    asm("MOV #0x0000,R11");
    asm("MOV #0x0000,R12");
    asm("MOV #0x0000,R13");
    asm("MOV #0x0000,R14");
    asm("MOV #0x0000,R15");
    asm("MOV #0x0000,SR");
    asm("MOV #0x2280,SP");
    asm("MOV #0xE006,PC");
}


void Clear_Watchdog(void){
  WDTCTL = WDTPW + WDTCNTCL; //清除WATCH DOG
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


unsigned char Init_UART()
{
  //r_Pipe_ID = Take_Pipe(IN_BUF_LEN);
  //t_Pipe_ID = Take_Pipe(OUT_BUF_LEN);
  P3SEL |= 0x30;                            // P3->4,5 = USART0 TXD/RXD
  ME1 |= UTXE0 + URXE0;                     // Enable USART0 TXD/RXD
  UCTL0 |= CHAR;                            // 8-bit character
  UTCTL0 |= SSEL1;                          // UCLK = SMCLK
  
  //UART0_SetBaudrate(9600);
  UART0_SetBaudrate(57600);
  
  IE1 |= URXIE0;	    	// enable transmission and receiver intrrupt
    
  return 1;
}


void SendText(unsigned char *SBUF,unsigned int length){
  for (int i=0;i<length;i++){
    while (!(IFG1 & UTXIFG0));                                   // USART0 TX buffer ready?
    TXBUF0 = SBUF[i];
  }
}

void ProcessUpdate(unsigned char RXBUF){
  if (UpdateP.BufFulltoWrite!=0) return;
  unsigned char RCVData;
  unsigned char Donothing=0;
  if(RXBUF>=0x30&&RXBUF<=0x39){ //--------------------------------0-9 ->0~9
    RCVData=RXBUF-0x30;
  }
  else if(RXBUF>=0x41&&RXBUF<=0x46){ //---------------------------A-F ->10~15
    RCVData=RXBUF-0x41+10;
  }
  else if(RXBUF==0x40){ //-----------------------------------------@ 收到後的後4個BYTE視成位址
    UpdateP.Address_mode=1;
    UpdateP.Address=0;
    UpdateP.Data=0;
    UpdateP.Address_counter=5;
  }
  else if(RXBUF=='q'||RXBUF=='Q'){ //-----------------------------離開UPDATE模式，並重啟
    Reset();
  }else{  //其他都不做事
    Donothing=1;
  }
  
  if(Donothing==0){ //如果有要做事
     //=========================================================================== 
    if (UpdateP.Address_mode==1){  //輸入為位址
      UpdateP.Address_counter--;
      //Address|=(((unsigned int)RCVData)<<(4*Address_counter));  //左移12,8,4,0
      UpdateP.Address|=LeftShift(RCVData,4*UpdateP.Address_counter);
      if(UpdateP.Address_counter==0){  //塞滿4BYTE->->離開ADDRESS MODE
        UpdateP.Address_mode=0;
        UpdateP.Data_counter =4;
        UpdateP.InputCount=0; //清空成0->->滿512BYTE才對FLASH作存取
//        printf("\nR: 0x%X ",0,(u16)UpdateP.Address);
        SendText((unsigned char *)AddressOK,9);
      }
    //=========================================================================== 
    }else{                 //輸入為DATA時
      if (UpdateP.Data_counter>1){ //塞滿4BYTE->->轉成資料
        UpdateP.Data_counter--;
        //Data|=(((unsigned int)RCVData)<<(4*Data_counter));  //左移12,8,4,0
        UpdateP.Data|=LeftShift(RCVData,4*UpdateP.Data_counter);
      }else{               //將資料寫入，並初始化變數，接收下4BYTE
        UpdateP.Data|=RCVData;
        UpdateP.BUF[UpdateP.InputCount]=(u8)((u16)UpdateP.Data>>8);
        UpdateP.BUF[UpdateP.InputCount+1]=(u8)(UpdateP.Data&0xff);
        //flash_write_word ((Data<<8)|(Data>>8),Address);
        UpdateP.Address+=2;
        UpdateP.InputCount+=2;
        UpdateP.Data_counter=4;
        //if (((Address-512)>=CodeBlockStart)&&(Address<CodeBlockEnd)&&(InputCount==512)){ //若在CODE區域，才更新
        if (((UpdateP.Address<=UpdateBlockStart)||((UpdateP.Address-512)>=UpdateBlockEnd))&&(UpdateP.InputCount==512)){ //若在CODE區域，才更新
           UpdateP.BufFulltoWrite=1; 
           //printf("W: 0x%X - 0x%X",0,(u16)(UpdateP.Address-512),UpdateP.Address);
        }
        UpdateP.Data=0; //要歸0->->否則下筆資料會錯
      }
      if (UpdateP.InputCount==512)SendText((unsigned char *)DataOK,6);
    }
  }
}

unsigned int LeftShift(unsigned int value,unsigned char power){
  for(int i=0;i<power;i++){value<<=1;}
  return value;
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
    //Clear_Watchdog();
    if((FBuf[i]!=0xff)||(FBuf[i+1]!=0xff)){  //如果是0xFF就不更新
      *Flash_ptr =((unsigned int)FBuf[i]<<8)|(FBuf[i+1]);                       // Write value to flash
    }
    Flash_ptr++;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void flash_erase_multi_segments (unsigned int Seg_Address,unsigned int Number_of_segments)
{
  unsigned int *Flash_Ptr;                      // Flash pointer
  unsigned int i;
  for (i=0;i<=Number_of_segments;i++){
     //Clear_Watchdog();
     Flash_Ptr = (unsigned int *) (Seg_Address + 512 * i) ;     // Initialize Flash pointer
     FCTL3 = FWKEY;                        // Clear Lock bit
     FCTL1 = FWKEY + ERASE;                // Set Erase bit
     *Flash_Ptr = 0;                       // Dummy write to erase Flash segment
     FCTL1 = FWKEY;                        // Clear Erase bit
     FCTL3 = FWKEY + LOCK;                 // Reset LOCK bit
  }
}

void CheckUpdatePacket(unsigned char RxData){
  if (RxData=='U'){           //收到第一個字是U就開始統計
    UpdateP.CheckUpdateIndex=0;
    for(int i=0;i<=7;i++)UpdateP.UpdatingPacket[i]=0x00;
  } 
  //開始統計後->->依序丟入DATA
  UpdateP.UpdatingPacket[UpdateP.CheckUpdateIndex]=RxData;
  UpdateP.CheckUpdateIndex++;

  if(UpdateP.CheckUpdateIndex==6){    //判斷收到的字是否是'Upload'
    char Same=1;
    for(int i=0;i<=5;i++){
      if (Upload_STR[i]!=UpdateP.UpdatingPacket[i])Same=0;
    }
    UpdateP.CheckUpdateIndex=0;
    for(int i=0;i<=7;i++)UpdateP.UpdatingPacket[i]=0x00;
    if(Same==1){
      UpdateP.Updating=1;
      flash_write_word(0xAAAA,0xFE00);
      Reset();
    }  //相同的話。進入更新模式
  }

}

//-------------------------------------------------------------------------------------------------------------------------
#define AWRN  BIT0  //R/W#
#define AA0   BIT1  //D/C#
#define RST   BIT2  //RESET#
#define ACSB  BIT3  //CS#
#define OLED_EN  BIT6  //OLED_ENABLE P5.6



void UP_OLED_Reset(){	//OLED Reset
  P5OUT &= ~RST;
  P5OUT |= RST;			
}

void UP_Dis_OLED(){
  P5OUT &= ~RST;
  P5OUT&=~OLED_EN;
}

void UP_En_OLED(){
  P5OUT&=~OLED_EN;
  UP_show_pic();
}

void UP_initial_ssd1303(){ 	//OLED初始化
  unsigned char k;
  P4DIR=0xFF;
  P5DIR=0xFF;
  P5OUT = 0x41;  		// 0100 0001  set BS1、BS2、RD to Hi
  UP_OLED_Reset();
  P5OUT &= ~AA0; 		
  P5OUT &= ~ACSB; 		// set ACSB(CS#) to low
  for(k=0;k<16;k++)
  {     	
    P4OUT = UP_initial[k];
    P5OUT &= ~AWRN; 		//set AWRN to low	
    P5OUT |= AWRN; 		
  }
}

void UP_set_col(){
  P4OUT=0x00;    		// Lower column
  P5OUT &= ~AWRN;
  P5OUT |= AWRN;

  P4OUT=0x10;   		// high column 0x60=96 , 0x1F=132
  P5OUT &= ~AWRN;
  P5OUT |= AWRN;
}

void UP_show_pic(){
  unsigned char page;
  u16 i,j;
  page=0xB0;  		
  for(i=0;i<1024;i+=128)  {
    P5OUT &= ~AA0;	  	
    P4OUT=page;           	//page number Setting
    P5OUT &= ~AWRN;	
    P5OUT |= AWRN;	
    UP_set_col();             	// setup low/high column
    P5OUT |= AA0;
    for(j=0;j<128;j++){	
      //page_data=pic[pic_index][j+i*128];
      //page_data=DC[j+(i<<7)];
      P4OUT=UP_DC[j+i];
      //P4OUT=0X03;
      P5OUT &= ~AWRN;	
      P5OUT |= AWRN;	 	
    }
    page++;
  }
}


void UP_Init_OLED(){
   //////// for OLED use ////////
  	P4DIR|=0xFF;				    		
  	P5DIR|=0xFF;    								
  	P5OUT = 0x41;  						// 01000001 set OLED_ENABLE,OLED_R/W#
  	UP_initial_ssd1303();
  	 						//clear picture
}

#pragma vector=UART0RX_VECTOR
__interrupt void usart0_rx (void){
  unsigned char RX;
  RX=RXBUF0;
  //printf("%c",RX);
  if(UpdateP.Updating==1){
    ProcessUpdate(RX);
  }else{
    UpdateP.DoNormalRxFunct(RX);
    CheckUpdatePacket(RX); //check the Update keyword
  }
}



//-------------------------------------------------------------------------------------------------------------------------
/*
void CloseAllInterrupt(){
  P5OUT &= ~ECGShutdown;
  P6OUT &= ~BeepPort;
  NAND_Close();
  ADC12CTL0 = 0x00;                     // Disable conversions
  ADC12CTL1 = 0x00;                     // Stopt conversion 
  ADC12MCTL1 = 0x00;
  ADC12IE = 0x00;
  TBCTL=0x00;
  P1IE=0x00;
  P2IE=0x00;
}
*/
/*
void UART0_SetBaudrate(unsigned long Baudrate){
  double Rate,Mul;
  unsigned char MCTL=0;
  UMCTL0=0;
  unsigned long Divider,Muldiv;
  Rate=(double)8000000/(double)Baudrate;
  Divider=Rate;
  UBR00=Divider;
  UBR10=Divider>>8;
  Rate-=(double)Divider;
  for (int i=1;i<=8;i++){
    Mul=Rate*(double)i;
    Muldiv=Mul;
    Mul-=(double)Muldiv;
    if (Mul>=0->5){MCTL=1;}else{MCTL=0;}
    //UMCTL0|=MCTL<<(i-1);
    UMCTL0|=LeftShift(MCTL,(i-1));
  }
  UCTL0 &= ~SWRST;
}
*/
