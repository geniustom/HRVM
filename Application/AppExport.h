
#ifndef APPUse_H

#define APPUse_H

#include <stdlib.h>
#include "../DriverLib/DriverExport.h"        // Standard Equations
#include "../Bootloader/BootloaderExport.h"        // Standard Equations
#include "../RTOS/RTOSExport.h"        // Standard Equations
#include "Menu/MenuUI.h"
#define max(x,y) (x)>(y)?(x):(y)
#define min(x,y) (x)<(y)?(x):(y)
//#define abs(x) (x)>0?(x):(-x)
#define swap(a,b) {int temp = a; a = b; b = temp;}
/**************************************************************************
                              計算SDNN相關
**************************************************************************/
#define SDNNSaveFlashAddr   0xDE00
#define SDNNSaveFlashSize   1280
extern const unsigned int RRIVmsSaveFlash[SDNNSaveFlashSize]@SDNNSaveFlashAddr; 
//存RRIV在內部的FLASH..RRIV的單位為ms-->RRIVms=RRIV*(1000/256)=RRIV*1000/256
typedef struct{   //Convolution coef Buffer
   unsigned int RRIVCount;
   unsigned int AVGRRIV;
} SDNN_Struct;
extern SDNN_Struct SDNNStruct;
extern void InitSDNNData();
extern void AddRRIVtoInterFlash(unsigned int RRIV);
extern void PrintSDNN();
/*************************************************************************
;                       ECG FILTER FUNCTION
*************************************************************************/
typedef struct{   //Convolution coef Buffer
   int *Coef;
   int NewValue;
   unsigned int Front;
   unsigned int len;
} CircultBuffer;

typedef struct{   //Convolution coef Buffer
   unsigned char *Coef;
   unsigned char NewValue;
   unsigned int Front;
   unsigned int len;
} CircultBuffer8BIT;

typedef struct{
   CircultBuffer CBUF;
   int BUF[64];
}ECGStructure;

typedef struct{
   CircultBuffer8BIT CBUF;
   char BUF[64];
}ECGStructure8BIT;


#define DiffThreshold 255
#define MAVariantThreshold 55   //變異度
#define PulseHighest 256
#define PulseLowest 30
#define ECGRate 256
#define K0init 800
#define MinDynamicThreshold 20
#define HRperMinRate  15360  //60*256 
#define ECGRawBackBufLen 180 //存RAW DATA時..備用BUF的大小(太小會當機)
#define ECGMeasureBound 76800

extern const int HP_Point33Hz[64];
extern const int LP_40Hz[64];


extern ECGStructure ORGECG;
extern int ORGBUF[64];
//extern ECGStructure BaselineECG;
//extern int BaselineBUF[64];
extern ECGStructure LP40ECG;
extern int LP40BUF[64];
extern CircultBuffer8BIT MAVariant;
extern unsigned char MABUF[64];
//extern CircultBuffer RRIVSquence;
//extern int RRIVBUF[64];
extern CircultBuffer8BIT DiffECG;
extern unsigned char DiffBUF[64];
//extern unsigned char *ECGRawBackBuf[ECGRawBackBufLen];

extern void CreateCoefBuffer(CircultBuffer *rb,unsigned int Len);
extern void CreateStaticCoefBuffer(CircultBuffer *rb,unsigned int Len,int *Databuff);
extern void CreateStaticCoefBuffer8BIT(CircultBuffer8BIT *rb,unsigned int Len,unsigned char *Databuff);
extern void CreateCoefBuffer8BIT(CircultBuffer8BIT *rb,unsigned int Len);
extern void ReleaseCoefBuffer(CircultBuffer *rb);
extern void ReleaseCoefBuffer8BIT(CircultBuffer8BIT *rb);

extern long HardwareMACS(int *Hn,int *Xn,unsigned int Len,unsigned int Front);
extern int InnerProduct(int *Hn,int *Xn,unsigned int Len,unsigned int Front);
extern long Convolution(CircultBuffer *rb,int const *Hn,int ADC_Value);
extern void PutDataNOConvolution(CircultBuffer *rb,int ADC_Value);
extern void PutDataOnly(CircultBuffer *rb,int ADC_Value);
extern void PutDataNOConvolution8BIT(CircultBuffer8BIT *rb,unsigned char ADC_Value);

extern void DiffFilter8BIT(CircultBuffer8BIT *rb,unsigned int ADC_Value);
extern int Differential(CircultBuffer *rb);
extern unsigned char MatchFilter8BIT(CircultBuffer8BIT *rb);
extern unsigned char DynamicThreshold(unsigned char MatchValue);
//extern unsigned char ProcessRRIVSquence();
//extern unsigned char RWaveDetected(CircultBuffer *rb);
extern unsigned char ComputeAvgHR(unsigned char HRvalue,unsigned char *EcgDetect);
extern void RTC_Show_Test_Time();
extern void Filter_Pulse(int *value);
extern int ECG_Filter(int value);
extern void ECGFinal();
/*************************************************************************
;                       ECG Structure and Prameter
*************************************************************************/
//#define ECG_time 5  
//ECG 搜尋範圍
#define ECG_range_min       28
#define ECG_range_max       245 
#define ECG_AVGSize         5
#define ECG_AVGSizeMin      3
#define Smooth_Val          16
#define Filter_Pulse_ref    35
typedef struct{
  /********************************************************************
                              計算RRIV相關
  ********************************************************************/
  unsigned char HRArray[ECG_AVGSize]; //計算RR的QUEUE的陣列
  unsigned char HRAvgIndex;           //計算RR的QUEUE的陣列索引
  unsigned long NoHRSampleDelay;      //偵測不到R坡時..要保留多久(SampleDealyBound)
  unsigned char AVGHR;                //算出平均後的HR
  unsigned char TimerID;              //目前的HR
  unsigned long RRIV;                 //目前取得的RRIV
  unsigned long RRIVOffset;           //取得相對於原始資料的最大值中..最高點的偏移
  unsigned long RRIVNextOffset;       //取得相對於原始資料的最大值中..最高點的偏移
  unsigned int  PointAverage[3];
  /********************************************************************
                              儲存ECG相關
  ********************************************************************/ 
  //unsigned char DataIsWritting; //若此時FLASH正在寫入..設為1,寫入完畢..清為0
  //unsigned char NeedSave; //若需要儲存時..設為1..儲存後..清為0  
  //unsigned char TempBUF[64]; //當ECG BUF被鎖住時..資料存在這裡
  unsigned int ECG_index;  //判定是否滿512..以便儲存用
  //unsigned int TempBufIndex;  //當FLASH正在寫入時..TEMP BUF用
  unsigned long MeasureTime;  //量測時間
  unsigned char ShowHRGraph;  //=1再下回合秀出愛心的圖..另一方面..也表示目前有抓到心跳
  unsigned char NoHRDelay;    //判定目前已經多久沒抓到心跳了
  unsigned char ECGFinal;
  unsigned int ORG_ECGData;   //原始AD完的DATA
  /********************************************************************
                              FILTER使用
  ********************************************************************/
  //int Filter_Pulse[3];
  int Smooth[Smooth_Val];
  unsigned int BaseLine;
  unsigned char DigitalGain;  //數位Gain 大小預設為1倍
  double T0;
  unsigned long K0;
  unsigned long Pulse;
  unsigned char RDetected; //本回合如果偵測到R波..回傳1
}ECGData;

extern ECGData ECGStruct;
extern long round(double x);
/********************************************************************
                            FLASH 存取資料結構
********************************************************************/
extern FlashStruct ECGRawFile;
/********************************************************************
                            TIME RTC 相關
********************************************************************/
#define RTCAddr0 0xE900
#define RTCAddr1 0xEB00
#define Page_RTC 1024
#define RTCSavePerISR 240  //30秒存一次
extern unsigned char RTCSaveCount;
extern void StopDateTime();
extern void Show_DateTime();
extern void SaveTime();
extern void LoadTime();
extern void SaveTimeToAll();
extern void SetTime(unsigned char YY1, unsigned char MM1, unsigned char DD1,unsigned char H1, unsigned char M1);
extern void ShowTimeInFlash();
/********************************************************************
                            MENU UI 相關
********************************************************************/
#define AutoShutdownTimeout 360 //閑置多久自動關機..單位為秒
extern void Draw_BG_Image();
extern void ShowDateTime_ConnPC();
extern void User_Entry();
extern void Check_RDMode(unsigned char btn_val);
extern MenuNode CurrentNode;    //目前所處的選單
extern unsigned char NowIsAtRDMode;
extern unsigned char PowerOffAfter5Sec;
/********************************************************************
                            Storage 相關
********************************************************************/
#define S_RTC_Page 1024
#define S_Ver_Page 1056
#define S_UserDataSize 256
extern const unsigned long UserStartPage[6];
extern const unsigned long UserEndPage[6];
extern const unsigned long UserIDPAGEList[6];

typedef struct{
  unsigned long Now_Index_page;
  unsigned long Start_page;
  unsigned long End_page;
  unsigned char EntryUser;
}UserDataStructure;

extern UserDataStructure UserDataStruct;
extern unsigned long Find_Start(unsigned char UserID);
extern unsigned char Save_UserDataCount(unsigned char UserID);
extern void Save_UserIndex(unsigned char UserID);
/********************************************************************
                            Uart 相關
********************************************************************/
#define CMDSize 10

typedef struct{   //Convolution coef Buffer
   unsigned char *DATABuf;
   unsigned int DataIndex;
   
   unsigned char *CMDBuf;
   unsigned int CMDIndex;
   
   unsigned char CMDMode;
   unsigned char CMDtodo;
   unsigned char CheckSum;
   
   unsigned int Datalength;
   unsigned char CMD_Type;
   unsigned long CMD_Address;
} CMDListener;

extern CMDListener Uart;
extern void InitUartStruct(unsigned char *CMDBUF,unsigned char *DATABUF);
extern void DoNormalRx(unsigned char RxData);
extern void DoUSBCMD();
extern unsigned char CMDBUF[11];


/*
extern void USB_CONNECT();
extern unsigned char RING_BUFF[517];				        //for data buffer use
typedef struct{
  unsigned char STATUS_INDEX;					//0 for command 1 for data 2 for ACK
  unsigned int INDEX;						//INDEX for command and data buff
  unsigned char ACK;						//byte for ACK check
  unsigned char DATA_TYPE,OPERATION_TYPE,USER_INDEX,DATA_INDEX;							
  unsigned char SERIAL_ID,DATA_LENGTH;
}USBStructure;
extern USBStructure USBStruct;
*/

#endif
