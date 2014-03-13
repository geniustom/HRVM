#ifndef RTOS_H
#define RTOS_H


#define MAX_TASK  5
#define PERIODIC  1
//#define SINGLE    0

struct SCHEDULAR{
    void (*pfunc)(void);
    unsigned char Delay;
    unsigned char Period;
    unsigned char Active;
};

#define ERROR_SCH_TASK_ARRAY_FULL   1




#define QueueSize 16

typedef struct{
  unsigned char PirorityValue;
  void  (*FunctionAddr)(void);
  unsigned char NodeID;
  unsigned char NodeType;
}PirorityNode;

typedef struct{
  PirorityNode Node[QueueSize+1];
  unsigned char PQueueSize;    //目前有的節點個數
  unsigned char NodeIndex;     //會在新增節點的時候累加..並ASSIGN給該NODE..每個NODE就會擁有一個獨立的ID
}PirorityQ;

extern PirorityQ TaskPQ;
extern PirorityNode NowTask;
extern void PQSched(PirorityQ *PQ);
extern void AddPQTask(PirorityQ *PQ,char PirorityValue,char NodeType,void (*Funct)());
extern void GetNodeType(PirorityNode *Result);


extern void Init_Device(void (*MainTaskFunctionPoint)());
extern void Init_RTOS();
extern void Run_RTOS(void (*BeforeSCH)(),unsigned char RunDelayTaskB,void (*AfterSCH)(),unsigned char RunDelayTaskA);
extern __no_init unsigned char USB[512]@0x2100;		   		//data buff
/**************************************************************************
                            RTOS 使用方式

  Init_Device(MainTaskFunctionPoint);  //初始化..並執行FUNCTION POINT的內容
  Init_RTOS();                         //OS初始化
  
           //RUN OS之前要做的其他初始化寫在這裡
  
  RunRTOS(beforeSCH,afterSCH);         //RUN RTOS(可將執行中要做的事寫成function point)
***************************************************************************/

#endif
