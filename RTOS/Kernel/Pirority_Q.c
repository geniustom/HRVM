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
  AssignNode(&PQ->Node[1],&x); //樹根節點        
  PirorityNode temp;
  AssignNode(&PQ->Node[PQ->PQueueSize],&temp);  //把最尾端的節點丟到TEMP
  int i=1,j=2;        //根節點,根節點的子節點        
  PQ->PQueueSize--;
  if(PQ->PQueueSize==0){
    return x;
  }else if(PQ->PQueueSize==1){
    AssignNode(&PQ->Node[2],&PQ->Node[1]);
    return x;
  }
  while(j<PQ->PQueueSize){         //從根節點開始往下調整
    if(j<QueueSize){   
      //確保右子結點有東西..然後從左或右節點中挑一個比較大的出來
      if(PQ->Node[j].PirorityValue<PQ->Node[j+1].PirorityValue){j++;}   
      //TEMP與A做比較,若結果需要調整將A與目前的節點作交換
      if(temp.PirorityValue<PQ->Node[j].PirorityValue){   
        AssignNode(&PQ->Node[j],&PQ->Node[i]);
        i=j;                  //下回合處理A位置的節點
      }
      j=j*2;                //改變兒子節點的指標
    }
    AssignNode(&temp,&PQ->Node[i]);
  }

  return x;
}

void PQinsert(PirorityQ *PQ,PirorityNode *Result){
  if(PQ->PQueueSize>=QueueSize){
    AddErrorCode(TaskQueueFull); //TASK滿了
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
//-------------向上調整--------------------

//-------------向下調整--------------------
}
*/
/*********************************************************************************************/
//                  Pirority Queue Schedualar Implement
//  1).NODE類型辨認..並給予一個FUNCTION POINT
//  2).取出工作並在ISR執行FUNCTION POINT
//  3).工作完成刪除該NODE
/*********************************************************************************************/

//--------------------取得目前位於ROOT的NODE----------------
void GetHiPirorityNode(PirorityQ *PQ,PirorityNode *Result){
  Result->PirorityValue=PQ->Node[1].PirorityValue;
  Result->FunctionAddr=PQ->Node[1].FunctionAddr;
  Result->NodeID=PQ->Node[1].NodeID;
  Result->NodeType=PQ->Node[1].NodeType;
}

//判定目前的NODE是屬於哪一類ISR.並給予一對映的FUNCTION POINT
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
//----------------將ISR MSG QUEUE的事件放到PirorityQueue中..並清空ISR MSG QUEUE-----------
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
//----------------選出一個優先權最高的排程並執行-----------
void PQSched(PirorityQ *PQ){
  ProcessISRMessage(PQ,&ISRMSG);
  
  if (PQ->PQueueSize==0)return;
  
  _DINT();
    PirorityNode Result;
    //GetHiPirorityNode(PQ,&Result);
    Result=PQdeleteMax(PQ);
    if (Result.NodeType!=0){GetNodeType(&Result);}
    AssignNode(&Result,&NowTask);  //將目前取出的TASK放入當前的TASK..以利其他程式或OS識別
  _EINT();
  
  if (Result.FunctionAddr!=0x00){Result.FunctionAddr();}  
}
//----------------加入一個具有優先權的行程-----------
void AddPQTask(PirorityQ *PQ,char PirorityValue,char NodeType,void (*Funct)()){
    PirorityNode Result;
    Result.PirorityValue=PirorityValue;
    Result.FunctionAddr=Funct;
    Result.NodeType=NodeType;
  _DINT();
    PQinsert(PQ,&Result);
  _EINT();
}
