
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
                              �p��SDNN����
**************************************************************************/
#define SDNNSaveFlashAddr   0xDE00
#define SDNNSaveFlashSize   1280
extern const unsigned int RRIVmsSaveFlash[SDNNSaveFlashSize]@SDNNSaveFlashAddr; 
//�sRRIV�b������FLASH..RRIV����쬰ms-->RRIVms=RRIV*(1000/256)=RRIV*1000/256
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
#define MAVariantThreshold 55   //�ܲ���
#define PulseHighest 256
#define PulseLowest 30
#define ECGRate 256
#define K0init 800
#define MinDynamicThreshold 20
#define HRperMinRate  15360  //60*256 
#define ECGRawBackBufLen 180 //�sRAW DATA��..�ƥ�BUF���j�p(�Ӥp�|���)
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
//ECG �j�M�d��
#define ECG_range_min       28
#define ECG_range_max       245 
#define ECG_AVGSize         5
#define ECG_AVGSizeMin      3
#define Smooth_Val          16
#define Filter_Pulse_ref    35
typedef struct{
  /********************************************************************
                              �p��RRIV����
  ********************************************************************/
  unsigned char HRArray[ECG_AVGSize]; //�p��RR��QUEUE���}�C
  unsigned char HRAvgIndex;           //�p��RR��QUEUE���}�C����
  unsigned long NoHRSampleDelay;      //��������R�Y��..�n�O�d�h�[(SampleDealyBound)
  unsigned char AVGHR;                //��X�����᪺HR
  unsigned char TimerID;              //�ثe��HR
  unsigned long RRIV;                 //�ثe���o��RRIV
  unsigned long RRIVOffset;           //���o�۹���l��ƪ��̤j�Ȥ�..�̰��I������
  unsigned long RRIVNextOffset;       //���o�۹���l��ƪ��̤j�Ȥ�..�̰��I������
  unsigned int  PointAverage[3];
  /********************************************************************
                              �x�sECG����
  ********************************************************************/ 
  //unsigned char DataIsWritting; //�Y����FLASH���b�g�J..�]��1,�g�J����..�M��0
  //unsigned char NeedSave; //�Y�ݭn�x�s��..�]��1..�x�s��..�M��0  
  //unsigned char TempBUF[64]; //��ECG BUF�Q����..��Ʀs�b�o��
  unsigned int ECG_index;  //�P�w�O�_��512..�H�K�x�s��
  //unsigned int TempBufIndex;  //��FLASH���b�g�J��..TEMP BUF��
  unsigned long MeasureTime;  //�q���ɶ�
  unsigned char ShowHRGraph;  //=1�A�U�^�X�q�X�R�ߪ���..�t�@�譱..�]��ܥثe�����߸�
  unsigned char NoHRDelay;    //�P�w�ثe�w�g�h�[�S���߸��F
  unsigned char ECGFinal;
  unsigned int ORG_ECGData;   //��lAD����DATA
  /********************************************************************
                              FILTER�ϥ�
  ********************************************************************/
  //int Filter_Pulse[3];
  int Smooth[Smooth_Val];
  unsigned int BaseLine;
  unsigned char DigitalGain;  //�Ʀ�Gain �j�p�w�]��1��
  double T0;
  unsigned long K0;
  unsigned long Pulse;
  unsigned char RDetected; //���^�X�p�G������R�i..�^��1
}ECGData;

extern ECGData ECGStruct;
extern long round(double x);
/********************************************************************
                            FLASH �s����Ƶ��c
********************************************************************/
extern FlashStruct ECGRawFile;
/********************************************************************
                            TIME RTC ����
********************************************************************/
#define RTCAddr0 0xE900
#define RTCAddr1 0xEB00
#define Page_RTC 1024
#define RTCSavePerISR 240  //30��s�@��
extern unsigned char RTCSaveCount;
extern void StopDateTime();
extern void Show_DateTime();
extern void SaveTime();
extern void LoadTime();
extern void SaveTimeToAll();
extern void SetTime(unsigned char YY1, unsigned char MM1, unsigned char DD1,unsigned char H1, unsigned char M1);
extern void ShowTimeInFlash();
/********************************************************************
                            MENU UI ����
********************************************************************/
#define AutoShutdownTimeout 360 //�~�m�h�[�۰�����..��쬰��
extern void Draw_BG_Image();
extern void ShowDateTime_ConnPC();
extern void User_Entry();
extern void Check_RDMode(unsigned char btn_val);
extern MenuNode CurrentNode;    //�ثe�ҳB�����
extern unsigned char NowIsAtRDMode;
extern unsigned char PowerOffAfter5Sec;
/********************************************************************
                            Storage ����
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
                            Uart ����
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
