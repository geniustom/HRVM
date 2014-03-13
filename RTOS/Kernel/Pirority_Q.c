#include "../RtosExport.h"
#include "../../DriverLib/DriverExport.h"        // Standard Equations


PirorityQ TaskPQ;
PirorityNode NowTask;

void AssignNode(PirorityNode *sou,PirorityNode *des){
  des->PirorityValue=sou->PirorityValue;
  des->FunctionAddr=sou->FunctionAddr;
  des->NodeID=sou->NodeID;
  des->NodeType=sou->NodeType;
}

PirorityNode PQdeleteMax(PirorityQ *PQ){
  PirorityNode x;
  AssignNode(&PQ->Node[1],&x); //��ڸ`�I        
  PirorityNode temp;
  AssignNode(&PQ->Node[PQ->PQueueSize],&temp);  //��̧��ݪ��`�I���TEMP
  int i=1,j=2;        //�ڸ`�I,�ڸ`�I���l�`�I        
  PQ->PQueueSize--;
  if(PQ->PQueueSize==0){
    return x;
  }else if(PQ->PQueueSize==1){
    AssignNode(&PQ->Node[2],&PQ->Node[1]);
    return x;
  }
  while(j<PQ->PQueueSize){         //�q�ڸ`�I�}�l���U�վ�
    if(j<QueueSize){   
      //�T�O�k�l���I���F��..�M��q���Υk�`�I���D�@�Ӥ���j���X��
      if(PQ->Node[j].PirorityValue<PQ->Node[j+1].PirorityValue){j++;}   
      //TEMP�PA�����,�Y���G�ݭn�վ�NA�P�ثe���`�I�@�洫
      if(temp.PirorityValue<PQ->Node[j].PirorityValue){   
        AssignNode(&PQ->Node[j],&PQ->Node[i]);
        i=j;                  //�U�^�X�B�zA��m���`�I
      }
      j=j*2;                //���ܨ�l�`�I������
    }
    AssignNode(&temp,&PQ->Node[i]);
  }

  return x;
}

void PQinsert(PirorityQ *PQ,PirorityNode *Result){
  if(PQ->PQueueSize>=QueueSize){
    AddErrorCode(TaskQueueFull); //TASK���F
  }
  PQ->PQueueSize++;
  PQ->NodeIndex++;
  Result->NodeID=PQ->NodeIndex;
  int i=PQ->PQueueSize;
  unsigned char ok=0;
  while(!ok){
    if (i==1) {
      ok=1;
    }else if (Result->PirorityValue<=PQ->Node[i/2].PirorityValue){
      ok=1;
    }else{
      AssignNode(&PQ->Node[i/2],&PQ->Node[i]);
      i=i/2;
    }
  }
  AssignNode(Result,&PQ->Node[i]);
}

/*
void AdjustPirority(PirorityQ *PQ,PirorityNode Node){
  if(PQ->PQueueSize==0)return;
//-------------�V�W�վ�--------------------

//-------------�V�U�վ�--------------------
}
*/
/*********************************************************************************************/
//                  Pirority Queue Schedualar Implement
//  1).NODE������{..�õ����@��FUNCTION POINT
//  2).���X�u�@�æbISR����FUNCTION POINT
//  3).�u�@�����R����NODE
/*********************************************************************************************/

//--------------------���o�ثe���ROOT��NODE----------------
void GetHiPirorityNode(PirorityQ *PQ,PirorityNode *Result){
  Result->PirorityValue=PQ->Node[1].PirorityValue;
  Result->FunctionAddr=PQ->Node[1].FunctionAddr;
  Result->NodeID=PQ->Node[1].NodeID;
  Result->NodeType=PQ->Node[1].NodeType;
}

//�P�w�ثe��NODE�O�ݩ���@��ISR.�õ����@��M��FUNCTION POINT
void GetNodeType(PirorityNode *Result){
  switch (Result->NodeType){
  case 0x01://--------------IO ISR--------------
    break;
  case 0x02://-------------RTC ISR--------------
    break;
  case 0x04://-------------WDT ISR--------------  
    break;
  case 0x08://-------------ADC ISR--------------
    break;
  case 0x10://--------------TB ISR--------------
    break;
  }
}
//----------------�NISR MSG QUEUE���ƥ���PirorityQueue��..�òM��ISR MSG QUEUE-----------
void ProcessISRMessage(PirorityQ *PQ,ISRMessageQueue *IM){
  _DINT();
  for(int i=0;i<IM->Index;i++){
    AddPQTask(PQ,
              IM->MSGNode[i].PirorityValue,
              IM->MSGNode[i].MSGType,
              IM->MSGNode[i].FunctionAddr
              );
  }
  ClearISRMessage(IM);
  _EINT();
}
//----------------��X�@���u���v�̰����Ƶ{�ð���-----------
void PQSched(PirorityQ *PQ){
  ProcessISRMessage(PQ,&ISRMSG);
  
  if (PQ->PQueueSize==0)return;
  
  _DINT();
    PirorityNode Result;
    //GetHiPirorityNode(PQ,&Result);
    Result=PQdeleteMax(PQ);
    if (Result.NodeType!=0){GetNodeType(&Result);}
    AssignNode(&Result,&NowTask);  //�N�ثe���X��TASK��J��e��TASK..�H�Q��L�{����OS�ѧO
  _EINT();
  
  if (Result.FunctionAddr!=0x00){Result.FunctionAddr();}  
}
//----------------�[�J�@�Ө㦳�u���v����{-----------
void AddPQTask(PirorityQ *PQ,char PirorityValue,char NodeType,void (*Funct)()){
    PirorityNode Result;
    Result.PirorityValue=PirorityValue;
    Result.FunctionAddr=Funct;
    Result.NodeType=NodeType;
  _DINT();
    PQinsert(PQ,&Result);
  _EINT();
}
