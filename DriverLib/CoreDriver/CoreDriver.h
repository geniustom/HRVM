#include "CoreDefine.h"


/*************************************************************************
;                          CPU FUNCTION
*************************************************************************/
extern void Use_XTAL2(void);
extern void Use_DCO(unsigned long CLK);
/*************************************************************************
;                          GPIO FUNCTION
*************************************************************************/
#define USBPIN BIT6
#define MaxOfHook 1   //需要HOOK的IO個數在這邊定義
typedef struct{
  unsigned int GPIOID;
  void (*HookCallBack)(void); //function pointer will be called after hook detect
  unsigned char HookType;  //HOOK種類 1:正緣觸發 2:負緣觸發 3:雙向觸發 
  unsigned long GPIOStartTime;
}GPIOHookStruct;
extern GPIOHookStruct GPIOHook[MaxOfHook];
extern void GPIODetect();
extern void AddGPIOHook(int GPIOID,unsigned char HookType,void (*HookCallBack)(),unsigned char HookIndex);
/*************************************************************************
;                          FLASH FUNCTION
*************************************************************************/
extern void  flash_write_word (unsigned int value,unsigned int address);
extern void  flash_write_Block (unsigned int address,unsigned char const *FBuf,unsigned int Length);
extern void  flash_erase_multi_segments (unsigned int Seg_Address,unsigned int Number_of_segments);
/*************************************************************************
;                          UART FUNCTION
*************************************************************************/
extern void OpenUart(unsigned char Port,unsigned long Baudrate,void (*CallBackFunction)(unsigned char));
extern void SendTextToUart(unsigned char Port,const unsigned char *SBUF,unsigned int length);
extern void SendByteToUart(unsigned char Port,unsigned char SBUF);
/*************************************************************************
;                          TIMERB FUNCTION
*************************************************************************/
#define TIMERB_AMOUNT  7
#define SINGLE      2   //只執行一次
#define NO_SERV  0xff
#define TIMERID char
#define TBADC     0x00
#define TB0       0x00
#define TB1       0x01
#define TB2       0x02
#define TB3       0x03
#define TB4       0x04
#define TB5       0x05
#define TB6       0x06
#define RANDOM    0xff
#define ERROR_NO_ERROR      0
#define ERROR_INITIAL       1
#define ERROR_CREAT_TIMER   2
#define ERROR_DELETE_TIMER  4
extern void Init_TB();
extern char TakeTimer(char RequiredTimer,unsigned int Interval,unsigned char mode,void (*f)());
extern unsigned char ReleaseTimer(char RequiredTimer);
extern void AdjustTimer(char num,unsigned int clk);
extern void TimerISRFunction(char num);
/*************************************************************************
;                          ADC MANAGER
*************************************************************************/
#define ADCChannel 3
#define ADQueueSize 16
typedef struct {
  unsigned int ADQueueData[ADQueueSize];
  unsigned char Front,Rear;
}ADC_Queue;

typedef struct {
  unsigned char TurnON;
  unsigned int ADIndex;
  unsigned char ADComplete;
  unsigned int ADCBUF[ADCChannel];
  unsigned char TimerID;
  void (*CallBackFP)(void);
  unsigned char NowCapChanel;
  ADC_Queue ADQueue;
}ADC_Struct;
extern ADC_Struct AD_Struct;

extern void ADCISR();
extern void TurnOnADC();
extern void SetADCCTL();
extern void Close_ADC();
extern void Open_ADC(void (*CallBackFP)(),unsigned char CapChanel);
extern unsigned int GetDataformADQueue(unsigned char ChanelID);
//傳入當ADC發生中斷時的回呼函式..包成ISR MSG送給OS..
//ADQ為當中斷發生時若ADQ不為0..會將DATA放進去..可以確保若沒被OS即時處理..仍能SAMPLE的正確性
extern void Init_DAC();
/*************************************************************************
;                          WDT TIMER FUNCTION
*************************************************************************/
extern void InitWDT();
extern unsigned long WDTCycle;