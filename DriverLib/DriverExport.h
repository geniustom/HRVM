#ifndef DriverUse_H

#include "CoreDriver/CoreDriver.h"
#define DriverUse_H

#define MSP430Clock 25000000
/*************************************************************************
;                       ISR MESSAGE PIRORITY
*************************************************************************/
#define ISRPIRORITY_TB            120
#define ISRPIRORITY_RTC           140
#define ISRPIRORITY_Button        140
#define ISRPIRORITY_ADC           150
#define ISRPIRORITY_UART          180
#define ISRPIRORITY_GPIOdetect    200
/*************************************************************************
;                          ISR MESSAGE
*************************************************************************/
#define ISRMSG_ADC                0x10

#define ISRMSG_TA0                0x20
#define ISRMSG_TA1                0x21
#define ISRMSG_TA2                0x22
#define ISRMSG_RTC_SEC            0x23

#define ISRMSG_TB0                0x30
#define ISRMSG_TB1                0x31
#define ISRMSG_TB2                0x32
#define ISRMSG_TB3                0x33
#define ISRMSG_TB4                0x34
#define ISRMSG_TB5                0x35
#define ISRMSG_TB6                0x36

#define ISRMSG_UART0RX            0x40

#define ISRMSG_LButtonPress       0x51
#define ISRMSG_RButtonPress       0x52
#define ISRMSG_CButtonPress       0x53
#define ISRMSG_LButtonLongPress   0x54
#define ISRMSG_RButtonLongPress   0x55
#define ISRMSG_CButtonLongPress   0x56
/*************************************************************************
;                          ISR MESSAGE FUNCTION
*************************************************************************/
#define MSGQueueSize 12
#define TACCR_0  4    //BUZZER
#define TACCR_1  32   //RTC trigger   1/1024 sec 約為1m sec 
#define TACCR_2  512  //BUTTON
#define TABuzzer 0    //TA1
#define TARTC  1      //TA0
#define TAButton 2    //TA2

typedef struct{
  unsigned char PirorityValue;
  void  (*FunctionAddr)(void);
  unsigned char MSGType;
}ISRMessage;

typedef struct{
  ISRMessage MSGNode[MSGQueueSize];
  unsigned char Index;
}ISRMessageQueue;

typedef struct{
  void  (*FunctionAddr)(void);   //只能是極短CPU時間的函式
  unsigned int Count;  //Count to Bount then call FunctionAddr at TAISR
  unsigned int Bound;  //delay ms
}DelayMessage;

extern ISRMessageQueue ISRMSG;
extern void AddISRMessage(ISRMessageQueue *IM,unsigned char PirorityValue,void (*F)(),unsigned char MSGType);
extern void ClearISRMessage(ISRMessageQueue *IM);
extern void Close_TA(unsigned char ID);
extern void Open_TA(unsigned char ID);
extern void Init_TA();
/*************************************************************************
;                          ERROR MESSAGE REPORT
*************************************************************************/
#define ISRQueueFull          BIT0
#define TaskQueueFull         BIT1
#define ADCQueueFull          BIT2
#define FlashBufFull          BIT3
#define InternalFlashErr      BIT4

extern unsigned int ERRORCODE;
extern void ShowErrorCode();
extern void AddErrorCode(unsigned int ERRORTYPE);
/*************************************************************************
;                          Button IO FUNCTION
*************************************************************************/
#define ButtonC BIT0
#define ButtonL BIT1
#define ButtonR BIT2

typedef struct{
  unsigned char PortNum;
  unsigned char Count;   //每一隻PIN的狀態維持了多久
  void (*Press)(void);   //function pointer will be called after short Press
  void (*LongPress)(void);   //function pointer will be called after long Press
  unsigned char PortSetting;  //每一隻PIN的中斷觸發種類 0:Hi觸發 1:Lo觸發
}ButtonStruct;
extern ButtonStruct ButtonLeft,ButtonRight,ButtonEnter;
extern void P2SetButtonIO(ButtonStruct *Button,unsigned char En,unsigned char PortNum,unsigned char PortSetting,void (*fP)(),void (*fLP)()); 
extern void CheckButton(ButtonStruct *Button);
extern void CheckButtonPress(ButtonStruct *Button);
extern void ButtonDetect();
/*************************************************************************
;                          OLED IO FUNCTION
*************************************************************************/
typedef struct 
{
  unsigned char Top;
  unsigned char Left;
  unsigned char Width;
  unsigned char Height;
  unsigned int MaxX;
  unsigned int MaxY;
  unsigned char Offset;  //The downsampling
  unsigned char NowX;
  unsigned char NowCount;
  unsigned char PreVal;
  unsigned char OrgAxis; 
}
OLEDCHART;
extern OLEDCHART OLC;
extern unsigned char DC[1024];
extern unsigned char RefreshPage[9];
extern const unsigned char OLED_Size8_Dig[12][8];
extern const unsigned char OLED_Size8_UpperWord[27][8];

extern void FullScreen();
extern void ClearScreen();
extern void Draw_Screen(unsigned char *Map);
extern void Init_OLED(void);
extern void Dis_OLED();
extern void En_OLED();
extern void show_pic();
extern void ShowPIC1Page();
extern void DrawLine(unsigned char StartX,unsigned char StartY,unsigned char EndX,unsigned char EndY);
extern void DrawBlock_Area(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char *BlockData);
extern void Full_Block(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char BlockData);
extern void DrawBlock(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char *BlockData);
extern void ClearBlock(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height);
extern void CreateChart(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned int MaxX,unsigned int MaxY,OLEDCHART *OLC);
extern void Print(unsigned char *CharData,unsigned char len,unsigned char Left,unsigned char Top,unsigned char fontsize);
extern void AddValue(OLEDCHART *OLC,int value);
/*************************************************************************
;                          NAND FLASH IO FUNCTION
*************************************************************************/
typedef struct {
  unsigned char Busy;           //busy for RW
  unsigned char NeedtoAccess;   //0:don't use 1:Read  2:Write
  unsigned int NowStep;         //if now is busy then do RW by step
  /***************************************************
    For Write   0:      erase if Page need
                1:      ECC Encode
                2:      SECC Encode
                3:      Write ECC to Flash
                4~515:  Write Data to Flash
    For Read    0:      Read ECC form Flash
                1:      ECC Decode
  `             2~513:  Read Data form Flash
                514:    ECC Encode form Flash
                515:    Read ECC form Flash
  ***************************************************/
  unsigned char *MainBuf;       //if MainBuf not full..save data to MainBuf 
  unsigned int MainBufIndex;
  unsigned int SpareIndex;
  unsigned char *BackupBuf;     //if MainBuf full..the MainBuf locked.. and save data to BackupBuf
  unsigned int BackupBufIndex;
  unsigned int BackLen;         //備用區的大小
  unsigned long AccessPage;
  unsigned char Sparebuff[16];
}FlashStruct;
extern void Init_Flash(void);
extern void Reset_Flash(void);
extern int BlockErase(unsigned long nAddr,unsigned char BusyWait);
extern void WriteByteToFlashBuf(FlashStruct *F,unsigned char Byte);
extern void FRead_Data(unsigned long Page_index,unsigned char *Databuff);
extern void FSave_Data(unsigned long Page_index,unsigned char *Databuff);
extern void InitFlashStruct(FlashStruct *F,unsigned char*MainBuf,unsigned char*BackupBuf,unsigned int BackLen);
extern void StepSave_Flash(FlashStruct *F);
extern void StepRead_Flash(FlashStruct *F);
//extern int WriteByte(unsigned int nCol,unsigned long nRow,unsigned char nValue);
//extern unsigned char ReadByte(unsigned int nCol,unsigned long nRow);
/*************************************************************************
;                          SD CARD function
*************************************************************************/
#include "ExternDriver/SDCard/HAL_SDCard.h"
/*************************************************************************
;                          BUZZER IO FUNCTION
*************************************************************************/
#define BuzzerPin BIT3

extern void BuzzerOn();
extern void BuzzerOff();
extern void Beep1Sec();
extern void ShortBeepSetTimes(unsigned char Times);
/*************************************************************************
;                          Power function
*************************************************************************/
#define BatteryArraySize 3
#define BatteryGridArraySize 20

extern void Open_BattDetect();
extern void Close_BattDetect();
extern void PowerOn();
extern void PowerDown();
extern void PowerDownLongTime();
typedef struct{
  unsigned char InitCount;        //從開始偵測的時候..就要把值塞進去了        
  unsigned char BattGrid;         //電池格數
  unsigned char BattID;
  unsigned char BattDetectState;  //=0目前功能關閉 =1目前功能開啟
  unsigned char BattIndex;        //BatteryArray的Index
  unsigned int  BatteryArray[BatteryArraySize];
  unsigned char BattGridIndex;    //BatteryGridArray的Index
  unsigned char BatteryGridArray[BatteryGridArraySize];
}BatteryStruct;
extern BatteryStruct BattStruct;
/*************************************************************************
;                          Real time clock
*************************************************************************/
typedef struct{
  void (*F)();  //當計數到秒的時候呼叫的事件
  unsigned int  mSecond;
  unsigned char  Second;
  unsigned char  Minute;
  unsigned char  Hour;
  unsigned char  TestSecond;
  unsigned char  TestMinute;
  unsigned int  NowDisCount;
  void (*DisCountTimeOutFP)(void);  //當倒數計時器數到0的時候呼叫的事件
}RTC;
typedef struct{
  unsigned char  Day;
  unsigned char  Month;
  unsigned char  Year;
}DateOBJ;

extern RTC RTC_Obj;
extern DateOBJ Date_Obj;

extern void Init_RTC();
extern void Start_RTC(void (*F)());
extern void Stop_RTC();
extern void Clear_Date();
extern void Clear_RTC();
extern void UpdateDate();
extern void UpdateRTC();
extern void UpdateMsec();
extern void SetDisCount(unsigned int Sec,void(*TimeOutFP)());
extern void UpdateRTCFromRTCChip();

/*************************************************************************
;                          I2C RTC CHIP FUNCTION
*************************************************************************/
extern void I2C_ReadTime(void);
extern void I2C_WriteTime(unsigned char Y,unsigned char M,unsigned char D,unsigned char h,unsigned char m,unsigned char s);
/*************************************************************************
;                          BASIC INCLUDE FUNCTION
*************************************************************************/
extern volatile void Init_Devicelib();


#endif
