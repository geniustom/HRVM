#include "../../DriverExport.h"    // Standard Equations

/**************************************************************************************
             RTC IC Function
**************************************************************************************/
#define SCL BIT6  //SCL--P6.6 
#define SDA BIT7  //SDA--P6.7

#define SCL_0 P6OUT&=~SCL
#define SDA_0 P6OUT&=~SDA
#define SCL_1 P6OUT|=SCL
#define SDA_1 P6OUT|=SDA
#define DIR_IN P6DIR&=~SDA
#define DIR_OUT P6DIR|=SDA
#define SDA_IN (P6IN & SDA)
#define SCL_OUT P6DIR|=SCL
#define SDA_OUT P6DIR|=SDA

//int Dataindex=0;
/*
void Delay(unsigned int j){
   unsigned int i;
   for(i=0;i<1*j; i++);
}
*/
void Start(void){
  SCL_OUT;
  SDA_OUT;
  SCL_0;
    SDA_1;
  SCL_1;
    //Delay(5);
    SDA_0;
    //Delay(5);
  SCL_0;
    SDA_1;
}
void SentStop(void)
{
  SCL_0;
    SDA_0;
  SCL_1;
    //Delay(5);
    SDA_1;
    //Delay(5);
  SCL_0;
}

char ReadByte(void)                 //i2c讀位元組
{
  int i;
  char Value=0; 
  DIR_IN;
    SCL_0;
    for(i=0;i<8;i++){
      SCL_1;
      //Delay(20);
      if(SDA_IN!=0){
        Value|=BIT0;
      }
      SCL_0;
      //Delay(20);
      if(i<7)Value=Value<<1;
    }
    SDA_0;
      DIR_OUT;
    SDA_1;
    //Delay(5);
    SCL_1;
    //Delay(5);
    SCL_0;
    //Delay(5);
    SentStop();
    //Delay(5);
    return Value;
}


unsigned char SentByte(char Value)        //i2c發送位元組
{
    unsigned int i;
    for(i=0;i<8;i++)
    {
    if((Value & BIT7)==0){
        SCL_0;
          //Delay(5);
          SDA_0;
          //Delay(5);
        SCL_1;
        //Delay(5);
    }else{
        SCL_0;
          //Delay(5);
          SDA_1;
          //Delay(5);
        SCL_1;
        //Delay(5);
    }
    Value=Value<<1;
    }
  SCL_0;
    //Delay(5);
    SDA_1;
    //Delay(5);
    DIR_IN;
  SCL_1;
  //Delay(5);
  if(SDA_IN==0){
    DIR_OUT;
    SCL_0;
    //Delay(5);
    return(1);
  }else{
    DIR_OUT;
    SCL_0;
    //Delay(5);
    return(0);
  }
}



/*函數名  ：WriteTo（），Address為所要寫入資料的位址，占兩個位元組，Data為所要寫如AT24C256的數據，*/
/*函數功能：將一個位元組的資料寫入固定位址*/
/*返回類型：如果寫入成功，返回值為1，否則返回數值為0*/
unsigned char WriteTo(char Address,char Data)     //向eeprom中保存位元組
{
  Start();
  if(SentByte(0xDE)==0)return(0);        //sent Write Command
  if(SentByte(Address)==0)return(0);
  if(SentByte(Data)==0)return(0);
  SentStop();
  //Delay(20);
  return(1);
}

/*函數名  ：ReadFrom（）Address為所要讀出資料的位址*/
/*功能    ：從AT24C256的固定位址讀出資料*/
/*返回類型：返回資料為字元型*/
char ReadFrom(char Address){
  Start();
  SentByte(0xDE);           //sent Write Command
  SentByte(Address);
  Start();
  SentByte(0xDF);           //sent Read Command
  return ReadByte();
}


unsigned char ByteToBCDCode(unsigned char Value){
  unsigned char BCD=(Value%10)+(Value/10*16);
  return BCD;
}

unsigned char BCDCodeToByte(unsigned char BCD){
  unsigned char Value=(BCD%16)+(BCD/16*10);
  return Value;
}

void I2C_WriteTime(unsigned char Y,unsigned char M,unsigned char D,unsigned char h,unsigned char m,unsigned char s){
  int ACK;
  ACK=WriteTo(0x08,0x0a);

  ACK=WriteTo(0x00,ByteToBCDCode(s));       //秒
  ACK=WriteTo(0x01,ByteToBCDCode(m));       //分
  ACK=WriteTo(0x02,0x80+ByteToBCDCode(h));  //時
  ACK=WriteTo(0x03,ByteToBCDCode(D));       //日
  ACK=WriteTo(0x04,ByteToBCDCode(M));       //月
  ACK=WriteTo(0x05,ByteToBCDCode(Y));       //年
  ACK=WriteTo(0x06,0x01);  //星期

  ACK=WriteTo(0x07,0x10);
  if (ACK==0)return;
}

void I2C_ReadTime(void){
  char Data[6];
  for(char i=0;i<6;i++){
    Data[i]=ReadFrom(i);
  }
  RTC_Obj.Second= BCDCodeToByte(Data[0]);
  RTC_Obj.Minute= BCDCodeToByte(Data[1]);
  RTC_Obj.Hour=   BCDCodeToByte(Data[2]-0x80);
  Date_Obj.Day=   BCDCodeToByte(Data[3]);
  Date_Obj.Month= BCDCodeToByte(Data[4]);
  Date_Obj.Year=  BCDCodeToByte(Data[5]);
}
