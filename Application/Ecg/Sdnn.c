#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"
#include "../Menu/MenuUI.h"
#include "math.h"

const unsigned int RRIVmsSaveFlash[SDNNSaveFlashSize]@SDNNSaveFlashAddr;
SDNN_Struct SDNNStruct;

void InitSDNNData(){
  //FCTL2 = FWKEY + FSSEL_2 + FN4 + FN2;      // SMCLK/20+1 = 400K for Flash Timing Generator
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^�[�F�H�W�t�׷|�ܺC,�YERASE���ѫh����ٲ����B�J
//------------------------------------------------------------------------Erease  
  for(unsigned int i=0;i<SDNNSaveFlashSize;i+=256){
    unsigned int *Flash_Ptr;                      // Flash pointer
    Flash_Ptr = (unsigned int *)SDNNSaveFlashAddr+i;     // Initialize Flash pointer
    FCTL3 = FWKEY;                        // Clear Lock bit
    FCTL1 = FWKEY + ERASE;                // Set Erase bit
    *Flash_Ptr = 0;                       // Dummy write to erase Flash segment
    FCTL1 = FWKEY;                        // Clear Erase bit
    FCTL3 = FWKEY + LOCK;                 // Reset LOCK bit
  }
  
  for(unsigned int i=0;i<SDNNSaveFlashSize;i++){
    if(RRIVmsSaveFlash[i]!=0xffff){
      AddErrorCode(InternalFlashErr); //Internal Flash Erase Error
    }
  }
  SDNNStruct.RRIVCount=0;
  SDNNStruct.AVGRRIV=0;
}
  
void AddRRIVtoInterFlash(unsigned int RRIV){
  unsigned int RRIVms=(unsigned int)(floor(((double)RRIV*1000)/256));  //RRIV*1000/256
  //-----------------------------------------------------------------------Write
  unsigned int *Flash_Ptr;
  Flash_Ptr = (unsigned int *)SDNNSaveFlashAddr+SDNNStruct.RRIVCount;
  FCTL2 = FWKEY + FSSEL_2 + FN4 + FN2;      // SMCLK/20+1 = 400K for Flash Timing Generator
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
    *Flash_Ptr = RRIVms;                    // Write value to flash
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit  
  //-----------------------------------------------------------------------
  if(RRIVmsSaveFlash[SDNNStruct.RRIVCount]!=RRIVms){  //�ˬd�g�J�O�_���T
    AddErrorCode(InternalFlashErr); //Internal Flash Erase Error
  }  
  SDNNStruct.RRIVCount++;
}  


void PrintSDNN(){
  //*******************************COMPUTE SDNN***************************/  
  //HRavg=AVGRRIVms
  unsigned int SDNN,HRAVG,VarRRIVms;
  unsigned long SumofRRIVms=0;
  //------------------------------------------�⥭��RRIVms(�Ĥ@��)
  for(unsigned int i=0;i<SDNNStruct.RRIVCount;i++){
    SumofRRIVms+=RRIVmsSaveFlash[i];
  }
  SDNNStruct.AVGRRIV=SumofRRIVms/SDNNStruct.RRIVCount; 
  //------------------------------------------�⥭��RRIVms(�ĤG��)
  SumofRRIVms=0;
  unsigned int CorrectRRIVCount=0;
  double VarofRRIVms=0;
  for(unsigned int i=0;i<SDNNStruct.RRIVCount;i++){
    VarRRIVms=RRIVmsSaveFlash[i]-SDNNStruct.AVGRRIV;
    if((VarRRIVms*VarRRIVms)<(SDNNStruct.AVGRRIV*SDNNStruct.AVGRRIV*0.25)){
      SumofRRIVms+=RRIVmsSaveFlash[i];
      CorrectRRIVCount++;
    }
  }
  SDNNStruct.AVGRRIV=SumofRRIVms/CorrectRRIVCount; 
  //------------------------------------------
  CorrectRRIVCount=SDNNStruct.RRIVCount;
  HRAVG=(unsigned int)(60000/SDNNStruct.AVGRRIV);
  for(unsigned int i=0;i<SDNNStruct.RRIVCount;i++){
    VarRRIVms=RRIVmsSaveFlash[i]-SDNNStruct.AVGRRIV;
    if((VarRRIVms*VarRRIVms)>(SDNNStruct.AVGRRIV*SDNNStruct.AVGRRIV*0.25)){         
                                                     //50%�зǮt
      CorrectRRIVCount--;                            //�L��RRIV
    }else{
      VarofRRIVms+=(double)(VarRRIVms*VarRRIVms);   //����RRIV
    }
  }
  SDNN=floor(sqrt(VarofRRIVms/(CorrectRRIVCount-1))*100);
  //**********************************************************************/
  Draw_BG_Image();
  Full_Block(30,17,68,50,0x00); 

  unsigned char SDNNDIGI[5];
  unsigned char K[12]={'0','1','2','3','4','5','6','7','8','9',':',' '}; 
  SDNNDIGI[0]=(unsigned char)SDNN/10000;  //�ʦ�
  SDNNDIGI[1]=(unsigned char)(SDNN/1000)%10; //�Q��
  SDNNDIGI[2]=(unsigned char)(SDNN/100)%10;  //�Ӧ�
  SDNNDIGI[3]=(unsigned char)(SDNN/10)%10; //�p�ƲĤ@��
  SDNNDIGI[4]=(unsigned char)(SDNN)%10; //�p�ƲĤG��
  
  if(SDNNDIGI[0]==0){SDNNDIGI[0]=11;}
  if(SDNNDIGI[4]==0){SDNNDIGI[4]=11;}	

  Print("S D N N",7,32,18,8);
  Print(&K[SDNNDIGI[0]],1,35,28,8);				
  Print(&K[SDNNDIGI[1]],1,45,28,8);				//:
  Print(&K[SDNNDIGI[2]],1,55,28,8);				
  Print(".",1,65,34,8);	
  Print(&K[SDNNDIGI[3]],1,75,28,8);		
  //Print(&K[SDNNDIGI[4]],1,85,28,8);	
  
  DrawLine(30,38,98,38);

  Print("  H R  ",7,32,42,8);
  Print(&K[HRAVG/100],1,47,52,8);				
  Print(&K[(HRAVG/10)%10],1,57,52,8);  
  Print(&K[(HRAVG%10)],1,67,52,8);	
  
  show_pic();
}

//-----------------------------------------------------------------------Write
/*
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  for(int i=0;i<6;i++){
    *Flash_Ptr = SaveTimeArray[i];                       // Write value to flash
    Flash_Ptr++;
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit  
*/

