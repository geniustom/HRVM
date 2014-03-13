#include "../../DriverLib/DriverExport.h"        // Standard Equations
#include "../../RTOS/RTOSExport.h"        // Standard Equations
#include "../AppExport.h"

USBStructure USBStruct;

unsigned char COMMAND_SELECT();
unsigned char DATA_SELECT();
unsigned char ACK_SELECT();
void Get_Memory_Index();
//void UART_SET();
void READ_DATA();
void WRITE_DATA();

//unsigned char COMMAND_BUFF[6]={0};			//for command buffer use
unsigned char STATUS_INDEX=0;					//0 for command 1 for data 2 for ACK
unsigned int INDEX=0;						//INDEX for command and data buff
unsigned char ACK=0;						//byte for ACK check
unsigned char DATA_TYPE,OPERATION_TYPE,USER_INDEX,DATA_INDEX;							
unsigned char SERIAL_ID=0,DATA_LENGTH=0;
unsigned long Ci,Ce, R_start, R_stop;


void USB_CONNECT()
{
  //UART_SET(); 
    
    if(STATUS_INDEX==1)
    {
      if(COMMAND_SELECT()==0xFF)	        	//status return 0xFF
      {
  	STATUS_INDEX=0;
  	INDEX=0;
      }
      DATA_TYPE=RING_BUFF[0];
      OPERATION_TYPE=RING_BUFF[1];
      USER_INDEX=RING_BUFF[2];
      DATA_INDEX=RING_BUFF[3];				//判斷command
      
      switch(OPERATION_TYPE)
      {
      case 0x10:
 	READ_DATA();					//read out data from FLASH
	STATUS_INDEX=0;
	INDEX=0;       
        break;
      case 0x20:
 	WRITE_DATA();    		   		//for test use write data into USB buff
	STATUS_INDEX=0;
	INDEX=0;       
        break;
      case 0x30:   //Sent Total Data count
        do{_NOP();}while((IFG1&&UTXIFG0)==0);           //wait UART buff ready
        U0TXBUF=Get_UserDataCount();     //Sent Total Data count
	STATUS_INDEX=0;
	INDEX=0;       
        break;
      case 0x31:   //Sent Total Data count
        Clear_UserDataCount(USER_INDEX); 
        break;
      default:
        
      STATUS_INDEX=0;
      INDEX=0;
      }
      ClearScreen();
    }    
}  	

//從PC的Command Package取得讀取的範圍
void Get_Memory_Index()
{
  //Ci = (64 + USER_INDEX) * 32;
 Ci=UserStartPage[USER_INDEX];
 Ce=UserEndPage[USER_INDEX];

  
  //將記憶體裡面的值讀出來
  FRead_Data(Ce,USB);
  
  R_start = USB[(DATA_INDEX<<4)+5] * 256 + USB[(DATA_INDEX<<4)+6] + Ci;
  R_stop = USB[(DATA_INDEX<<4)+7] * 256 + USB[(DATA_INDEX<<4)+8] + Ci;  
}

void READ_DATA(void){
    unsigned char Checksum, Send_Check;

    Ci=UserStartPage[USER_INDEX];
    Ce=UserEndPage[USER_INDEX];
     
     switch(DATA_TYPE){
     case 0x04:               
                  // Read total data count
       return;
       break;
     case 0x03:               
       Reset();            // Reset System
       return;
       break;
     case 0x02:        
       BlockErase(Ce,1);            // Clear User Test Index Data
       return;      
       break;
     case 0x10:                   // Get ECG Data  
       Get_Memory_Index();
       break;
     case 0x80:                   // Get Data Index    
       R_start = Ce;
       R_stop = Ce + 1;      
       break;        
     default:
       return;       
     }
  
     // 將倒數時間重新歸零
     //Clear_Test_RTC();
  
     Ci = R_start;
     Send_Check = 0xFF;
     while(Ci <= R_stop)
     {           
       if (Send_Check == 0xFF)
       {
          FRead_Data(Ci,RING_BUFF);
          if((RING_BUFF[0]==0xFF)&&(RING_BUFF[2]==0xFF)&&(RING_BUFF[4]==0xFF)&&(RING_BUFF[6]==0xFF)&&(RING_BUFF[8]==0xFF)){_NOP();}
          //testff=R_BUFF[0];
          Send_Check = 0x00;
          
          /*
          //////// move data from DC BUFF to RING_BUFF ////////	
          Checksum=0;
          for(j=0;j<513;j++)          // Old is 513 Date:10/13 Allen
          {
            Checksum=Checksum^R_BUFF[j];       //create Checksum code
          }
          */
          ////// send data to PC end //////
          do{
            _NOP();
          }while((IFG1&&UTXIFG0)==0);           //wait UART buff ready
          U0TXBUF=0XFE;
          
          do{
            _NOP();
          }while((IFG1&&UTXIFG0)==0); //wait UART buff ready		

          U0TXBUF=DATA_TYPE;              //send DATA_TYPE
  //      U0TXBUF=0X10;//send DATA_TYPE
          
          do{
            _NOP();
          }while((IFG1&&UTXIFG0)==0);//wait UART buff ready		
          
          U0TXBUF=Ci - R_start;
          //U0TXBUF=0X00;                 //send number index(SERIAL_ID)
  
          do{
            _NOP();
          }while((IFG1&&UTXIFG0)==0);//wait UART buff ready
          U0TXBUF=R_stop - R_start;         //send total number(DATA_LENGTH)
          //U0TXBUF=0X02;                 //send total number(DATA_LENGTH)
          
          Checksum=0;
          for(int j=0;j<512;j++)		//0~511共512筆資料
          {
            do
            {_NOP();}		        //wait UART buff ready
            while((IFG1&&UTXIFG0)==0);
            
            Checksum=Checksum^RING_BUFF[j];       //create Checksum code
            
            U0TXBUF=RING_BUFF[j];         //send data
            //U0TXBUF=0x31;//send data
          }
          _NOP();
          _NOP();
          
          do
          {_NOP();}			//wait UART buff ready
          while((IFG1&&UTXIFG0)==0);
          U0TXBUF=Checksum;		//send Checksum
              
          STATUS_INDEX=0;
          INDEX=0;                   
       }                
       // 等待ACK的回覆
       if (STATUS_INDEX==3)
       {
         //如果回覆PASS則開始送下一個封包
         if(RING_BUFF[1] == 0x00) 
         {
           Ci=Ci+1;
           //重送一次，將Check拉成0xFF
           Send_Check = 0xFF;
         }       
       }
     }
   }
/////////////////////////////////////////////////////////////////
//	function name:unsigned char WRITE_DATA()	   	       //	
//	function:write data to FLASH			       //
//      input:Page_index-->assign which page want to write     //
//      Databuff-->the buff which save the read out data       //
//	return:0x00 for read pass, 0xFF for read fail          //
/////////////////////////////////////////////////////////////////  	
void WRITE_DATA(void)
{
  unsigned long Page_I = 0;
  unsigned char STATUS=0;

//////// wait for DATA input ////////	
	STATUS_INDEX=0;
	INDEX=0;
        
WAIT_DATA:  	
  	do
  	{_NOP();}
  	while(STATUS_INDEX != 2);								
	if(DATA_SELECT()==0xFF)
	{
	  STATUS_INDEX=0;
	  INDEX=0;
	  goto WAIT_DATA;		        //ACK fail
	}
        
	DATA_TYPE=RING_BUFF[0];
	SERIAL_ID=RING_BUFF[1];
	DATA_LENGTH=RING_BUFF[2];
////////////////////////////////////////////////////////////////	
// 	Write data into Flash				      //
//      use by DATA_TYPE,USER_INDEX,DATA_INDEX                //
//////////////////////////////////////////////////////////////// 	
	//
    //to do

   //////// 將RING_BUFF內之資料寫入DC內 ////////	
	for(int i=3;i<515;i++)
	{
	  USB[i-3]=RING_BUFF[i];
	}
   //////// send ACK ////////	
	do
	{_NOP();}				//wait UART buff ready
	while((IFG1&&UTXIFG0)==0);
	U0TXBUF=0xFD;
        
	do
	{_NOP();}				//wait UART buff ready
	while((IFG1&&UTXIFG0)==0);
	U0TXBUF=0x00;				//send ACK pass
        
         //////// 判斷是否完畢 ////////
        if(SERIAL_ID!=DATA_LENGTH)
        {
              STATUS_INDEX=0;
              INDEX=0;
              goto WAIT_DATA;				//not yet
        }
        unsigned long PAGE_I=1536;
        switch(DATA_TYPE)
        {
        case 0x20:                // Set Time Dat   
          BlockErase(PAGE_I,1);
          for(int i=0;i<1000;i++){_NOP();}
          //RING_BUFF[0]=USB[0];
          //RING_BUFF[1]=USB[1];
          //RING_BUFF[2]=USB[2];
          //RING_BUFF[3]=USB[3];
          //RING_BUFF[4]=USB[4];
          FSave_Data(PAGE_I,USB);
          //FRead_Data(Page_I,R_BUFF);
          SetTime(USB[0],USB[1],USB[2],USB[3],USB[4]);
          SaveTimeToAll();
          break;
          //Reset_System();  //若USB插或拔..就RESET
        //default:          
        }                 
  	STATUS_INDEX=0;
  	INDEX=0;				//write over
}

/////////////////////////////////////////////////////////
//	function name:unsigned char COMMAND_SELECT()   	       //	
//	function:receiver command and check ACK        //
//	return:0x00 for ACK pass, 0xFF for ACK fail    //
/////////////////////////////////////////////////////////  	
unsigned char COMMAND_SELECT()
  {
	unsigned char STATUS;
	do
	{_NOP();}			//wait COMMAND transmission complete, if over 3s return fail
	while(INDEX<6);			//save from byte2~byte6
	ACK=0;
	for(int i=0;i<4;i++)
	  {ACK=ACK^RING_BUFF[i];}	//caculate checksum
	if(ACK!=RING_BUFF[4])
	{
//////// ACK FAIL ////////------------------------------------------
	  do
	  {_NOP();}			//wait UART buff ready
	  while((IFG1&&UTXIFG0)==0);
	  U0TXBUF=0XFD;
          
	  do
	  {_NOP();}				
	  while((IFG1&&UTXIFG0)==0);
	  U0TXBUF=0XFF;			//send ACK fail
          
	  return(STATUS|0xFF);		//return fail need to transmission again
	}
//////// ACK PASS ////////------------------------------------------
	do
	{_NOP();}			//wait UART buff ready
	while((IFG1&&UTXIFG0)==0);
	U0TXBUF=0XFD;
        
	do
	{_NOP();}				
	while((IFG1&&UTXIFG0)==0);
	U0TXBUF=0X00;			//send ACK pass
        
	return(STATUS&0x00);
	//////////////////////////////////
  }	


/////////////////////////////////////////////////////////
//	function name:unsigned char DATA_SELECT()	       //	
//	function:receiver DATA and check ACK           //
//	return:0x00 for ACK pass, 0xFF for ACK fail    //
/////////////////////////////////////////////////////////  	
unsigned char DATA_SELECT()
  {
	unsigned char STATUS;
	do
	{_NOP();}
	while(INDEX<4);       	        //do until get data length informaction
        
	do
	{_NOP();}		        //wait until data transmission complete, if over 3s return fail
	while(INDEX<517);	
        
	ACK=0;			        //save from byte2 to end
	for(int i=3;i<516;i++)
	  {ACK=ACK^RING_BUFF[i];}       //caculate checksum
        
	if(ACK!=RING_BUFF[516])
	{
	  //////// ACK FAIL ////////
	  do
	  {_NOP();}			//wait UART buff ready
	  while((IFG1&&UTXIFG0)==0);
	  U0TXBUF=0XFD;
          
	  do
	  {_NOP();}				
	  while((IFG1&&UTXIFG0)==0);
	  U0TXBUF=0XFF;			//send ACK fail
          
	  return(STATUS|0xFF);		//return fail need to transmission again	
	}
	//////// ACK PASS ////////
	return(STATUS&0x00);		//return pass	
  }


/////////////////////////////////////////////////////////
//	function name:unsigned char ACK_SELECT()	       //	
//	function:receiver ACK   		       //
//	return:0x00 for ACK pass, 0xFF for ACK fail    //
/////////////////////////////////////////////////////////
unsigned char ACK_SELECT()
{
  unsigned char STATUS;
  do
  {_NOP();}
  while(INDEX<2);			//wait until ACK transmission complete, if over 3s return fail
  if(RING_BUFF[0]==0)		//save only byte2	
  {
    return(STATUS&0x00);		//pass
  }
  return(STATUS|0xFF);		//fail
}


//////////////////////////////////////////////////////////////////////////////
//	function name:UART_SET()		       			    //	
//	function:set UART function UCLK is SMCLK=1Mhz, baurdrate is 9600    //
//		 datalength is 8BITS, one stop bit, parity is disable	    //
//	return:NaN		    					    //
//////////////////////////////////////////////////////////////////////////////
/*
void UART_SET(void)
{
  P3SEL = 0X30;			    		// SET PORT3 TO UART FUNCTION
  P3DIR = 0X10;			    		// set P3.4(TX) to output
  UCTL0 = 0X10;                     		// 8-bit UART mode
  UTCTL0 |= SSEL0;            	    		// UCLK is ACLK
  UBR00 = 0x03;                     		// ------
  UBR10 = 0x00;                     		// |-------> set baurdrate is 9600
  UMCTL0 = 0x4A;				// ------
  ME1 |= UTXE0 + URXE0;             		// allow to transceiver and receiver
  IE1 |= URXIE0;//UTXIE0 + URXIE0;	    	// enable transmission and receiver intrrupt
}
*/

/*
////////////////////////////////////////////////
//       UART interrupt service routine       //
////////////////////////////////////////////////
#pragma vector = USART0TX_VECTOR    			// 傳送中斷起始位置
__interrupt void USART0TX_ISR (void)
 {
 	_NOP();
 }
*/

void RXInNormalMODE(unsigned char RxData){
  if((URCTL0&0x20)==0x20) 
    _NOP();                             //receiver overflow 
  else{ 
    switch(STATUS_INDEX){
    case 1:                                   //Data Type: RING_BUFF full
      RING_BUFF[INDEX-1]=RxData;              //Data Type: Write Data to RING_BUFF
      INDEX++;
      break;     
    case 2:                                  //CMD Type: Get CMD Packet information
      if(INDEX==1)     
   	DATA_TYPE=RxData;
      else if(INDEX==2)
   	SERIAL_ID=RxData;
      else if(INDEX==3){
   	DATA_LENGTH=RxData;
   	RING_BUFF[0]=DATA_TYPE;
   	RING_BUFF[1]=SERIAL_ID;
   	RING_BUFF[2]=DATA_LENGTH;
      }else{
   	RING_BUFF[INDEX-1]=RxData;
      }
     
      INDEX++;
      break;
    case 3:                               
      RING_BUFF[1]=RxData;              //Data Type: Start Receive
      INDEX++;     
      break;
    default:
      if(RxData==0XFF){
 	STATUS_INDEX=1;			//indicate command status
 	INDEX = 1;
      }	
      else if(RxData==0XFE){
 	STATUS_INDEX=2;			//indicate data status
 	INDEX = 1;
      }	
      else if(RxData==0XFD){
 	STATUS_INDEX=3;			//indicate ACK status
 	INDEX = 1;
      }
    }   
  }  
}


void DoNormalRx(unsigned char RxData){
  RXInNormalMODE(RxData);
}



