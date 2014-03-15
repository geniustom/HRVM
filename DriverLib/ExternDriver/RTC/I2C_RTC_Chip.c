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

char ReadByte(void)                 //i2cŪ�줸��
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


unsigned char SentByte(char Value)        //i2c�o�e�줸��
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



/*��ƦW  �GWriteTo�]�^�AAddress���ҭn�g�J��ƪ���}�A�e��Ӧ줸�աAData���ҭn�g�pAT24C256���ƾڡA*/
/*��ƥ\��G�N�@�Ӧ줸�ժ���Ƽg�J�T�w��}*/
/*��^�����G�p�G�g�J���\�A��^�Ȭ�1�A�_�h��^�ƭȬ�0*/
unsigned char WriteTo(char Address,char Data)     //�Veeprom���O�s�줸��
{
  Start();
  if(SentByte(0xDE)==0)return(0);        //sent Write Command
  if(SentByte(Address)==0)return(0);
  if(SentByte(Data)==0)return(0);
  SentStop();
  //Delay(20);
  return(1);
}

/*��ƦW  �GReadFrom�]�^Address���ҭnŪ�X��ƪ���}*/
/*�\��    �G�qAT24C256���T�w��}Ū�X���*/
/*��^�����G��^��Ƭ��r����*/
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

  ACK=WriteTo(0x00,ByteToBCDCode(s));       //��
  ACK=WriteTo(0x01,ByteToBCDCode(m));       //��
  ACK=WriteTo(0x02,0x80+ByteToBCDCode(h));  //��
  ACK=WriteTo(0x03,ByteToBCDCode(D));       //��
  ACK=WriteTo(0x04,ByteToBCDCode(M));       //��
  ACK=WriteTo(0x05,ByteToBCDCode(Y));       //�~
  ACK=WriteTo(0x06,0x01);  //�P��

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
