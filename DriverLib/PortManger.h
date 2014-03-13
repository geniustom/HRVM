#include "DriverExport.h"

#define PORT1 0x100
#define PORT2 0x200
#define PORT3 0x300
#define PORT4 0x400
#define PORT5 0x500
#define PORT6 0x600

#define P1BIT0 0x100 | BIT0
#define P1BIT1 0x100 | BIT1
#define P1BIT2 0x100 | BIT2
#define P1BIT3 0x100 | BIT3
#define P1BIT4 0x100 | BIT4
#define P1BIT5 0x100 | BIT5
#define P1BIT6 0x100 | BIT6
#define P1BIT7 0x100 | BIT7

#define P2BIT0 0x200 | BIT0
#define P2BIT1 0x200 | BIT1
#define P2BIT2 0x200 | BIT2
#define P2BIT3 0x200 | BIT3
#define P2BIT4 0x200 | BIT4
#define P2BIT5 0x200 | BIT5
#define P2BIT6 0x200 | BIT6
#define P2BIT7 0x200 | BIT7

#define P3BIT0 0x300 | BIT0
#define P3BIT1 0x300 | BIT1
#define P3BIT2 0x300 | BIT2
#define P3BIT3 0x300 | BIT3
#define P3BIT4 0x300 | BIT4
#define P3BIT5 0x300 | BIT5
#define P3BIT6 0x300 | BIT6
#define P3BIT7 0x300 | BIT7

#define P4BIT0 0x400 | BIT0
#define P4BIT1 0x400 | BIT1
#define P4BIT2 0x400 | BIT2
#define P4BIT3 0x400 | BIT3
#define P4BIT4 0x400 | BIT4
#define P4BIT5 0x400 | BIT5
#define P4BIT6 0x400 | BIT6
#define P4BIT7 0x400 | BIT7

#define P5BIT0 0x500 | BIT0
#define P5BIT1 0x500 | BIT1
#define P5BIT2 0x500 | BIT2
#define P5BIT3 0x500 | BIT3
#define P5BIT4 0x500 | BIT4
#define P5BIT5 0x500 | BIT5
#define P5BIT6 0x500 | BIT6
#define P5BIT7 0x500 | BIT7

#define P6BIT0 0x600 | BIT0
#define P6BIT1 0x600 | BIT1
#define P6BIT2 0x600 | BIT2
#define P6BIT3 0x600 | BIT3
#define P6BIT4 0x600 | BIT4
#define P6BIT5 0x600 | BIT5
#define P6BIT6 0x600 | BIT6
#define P6BIT7 0x600 | BIT7

extern void SelectSpecialPins(int portnum);   //PXSEL|=portnum
extern void SelectGPIOPins(int portnum);  //PXSEL&=~portnum
extern void SetGPIOInput(int portnum);  //PXDIR&=~portnum
extern void SetGPIOOutput(int portnum); //PXDIR|=portnum
extern void OutputGPIOHi(int portnum);  //PXOUT|=portnum
extern void OutputGPIOLo(int portnum);  //PXOUT&=~portnum
extern void OutputGPIO(int portnum);   //PXOUT=portnum
extern char GetGPIO(int portnum);


extern void SetButtonTimer();
extern void P2SetButtonIO(ButtonStruct *Button,unsigned char En,unsigned char PortNum,unsigned char PortSetting,void (*fP)(),void (*fLP)());
extern void CheckButtonPress(ButtonStruct *Button);
extern void CheckButton(ButtonStruct *Button);
extern ButtonStruct ButtonLeft,ButtonRight,ButtonEnter;
