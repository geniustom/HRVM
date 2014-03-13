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
  unsigned char PQueueSize;    //�ثe�����`�I�Ӽ�
  unsigned char NodeIndex;     //�|�b�s�W�`�I���ɭԲ֥[..��ASSIGN����NODE..�C��NODE�N�|�֦��@�ӿW�ߪ�ID
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
                            RTOS �ϥΤ覡

  Init_Device(MainTaskFunctionPoint);  //��l��..�ð���FUNCTION POINT�����e
  Init_RTOS();                         //OS��l��
  
           //RUN OS���e�n������L��l�Ƽg�b�o��
  
  RunRTOS(beforeSCH,afterSCH);         //RUN RTOS(�i�N���椤�n�����Ƽg��function point)
***************************************************************************/

#endif
