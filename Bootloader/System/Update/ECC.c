#include "Flash.h"
#include "PortManager.h"



/////////////////////////////////////////////////////////////////////
//          main data area ECC generator			   //
/////////////////////////////////////////////////////////////////////
void ECC_gen(dataWidth *Buffer, dataWidth *ECC_code)
{
 u16 i;
 u8 data,bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7;
 u8 LP0=0,LP1=0,LP2=0,LP3=0,LP4=0,LP5=0,LP6=0,LP7=0,LP8=0,LP9=0,LP10=0,LP11=0,LP12=0,LP13=0,LP14=0,LP15=0,LP16=0,LP17=0;
 u8 CP0=0,CP1=0,CP2=0,CP3=0,CP4=0,CP5=0;
 for(i=0;i<512;i++)
 {
   data=Buffer[i];
   bit0=data&(0x01);
   bit1=data&(0x02);
   bit2=data&(0x04);
   bit3=data&(0x08);
   bit4=data&(0x10);
   bit5=data&(0x20);
   bit6=data&(0x40);
   bit7=data&(0x80);
   if (i&0x01)//1¡B3¡B5...
   {
     LP1=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP1);
   }
   else	
   {
     LP0=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP1);
   }
   if (i&0x02)//2 3¡B6 7
   {
     LP3=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP3);
   }
   else
   {
     LP2=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP2);
   }

   if (i&0x04)//
   {
     LP5=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP5);
   }
   else
   {
     LP4=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP4);
   }
   if (i&0x08)//
   {
     LP7=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP7);
   }
   else
   {
     LP6=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP6);
   }
   if (i&0x10)//
   {
     LP9=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP9);
   }
   else
   {
     LP8=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP8);
   }
   if (i&0x20)//
   {
     LP11=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP11);
   }
   else
   {
     LP10=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP10);
   }
   if (i&0x40)//
   {
     LP13=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP13);
   }
   else
   {
     LP12=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP12);
   }
   if (i&0x80)//
   {
     LP15=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP15);
   }
   else
   {
     LP14=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP14);
   }
   if (i&0xA0)//
   {
     LP17=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP17);
   }
   else
   {
     LP16=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^LP16);
   }
   CP0=(bit6^bit4^bit2^bit0^CP0);
   CP1=(bit7^bit5^bit3^bit1^CP1);
   CP2=(bit5^bit4^bit1^bit0^CP2);
   CP3=(bit7^bit6^bit3^bit2^CP3);
   CP4=(bit3^bit2^bit1^bit0^CP4);
   CP5=(bit7^bit6^bit5^bit4^CP5);
 }
 ECC_code[6]=LP0|LP1|LP2|LP3|LP4|LP5|LP6|LP7;
 ECC_code[7]=LP8|LP9|LP10|LP11|LP12|LP13|LP14|LP15;
 ECC_code[8]=LP16|LP17|CP0|CP1|CP2|CP3|CP4|CP5;
}
/////////////////////////////////////////////////////////////////////
//     spare area ECC generator			 	           //
/////////////////////////////////////////////////////////////////////
void S_ECC_gen(u8 *Buffer, u8 *ECC_code)
{
 u16 i;
 u8 data,bit0,bit1,bit2,bit3,bit4,bit5,bit6,bit7;
 u8 S_LP0=0,S_LP1=0,S_LP2=0,S_LP3=0;
 u8 S_CP0=0,S_CP1=0,S_CP2=0,S_CP3=0,S_CP4=0,S_CP5=0;
 for(i=0;i<16;i++)
 {
   data=Buffer[i];
   bit0=data&(0x01);
   bit1=data&(0x02);
   bit2=data&(0x04);
   bit3=data&(0x08);
   bit4=data&(0x10);
   bit5=data&(0x20);
   bit6=data&(0x40);
   bit7=data&(0x80);
   if (i&0x01)//1¡B3¡B5...
   {
     S_LP1=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^S_LP1);
   }
   else
   {
     S_LP0=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^S_LP1);
   }
   if (i&0x02)//2 3¡B6 7
   {
     S_LP3=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^S_LP3);
   }
   else
   {
     S_LP2=(bit7^bit6^bit5^bit4^bit3^bit2^bit1^bit0^S_LP2);
   }
   S_CP0=(bit6^bit4^bit2^bit0^S_CP0);
   S_CP1=(bit7^bit5^bit3^bit1^S_CP1);
   S_CP2=(bit5^bit4^bit1^bit0^S_CP2);
   S_CP3=(bit7^bit6^bit3^bit2^S_CP3);
   S_CP4=(bit3^bit2^bit1^bit0^S_CP4);
   S_CP5=(bit7^bit6^bit5^bit4^S_CP5);	
 }
 ECC_code[9]=S_LP0|S_LP1|S_LP2|S_LP3|S_CP0|S_CP1|S_CP2|S_CP3;
 ECC_code[10]=S_CP4|S_CP5|0XFC;
}

