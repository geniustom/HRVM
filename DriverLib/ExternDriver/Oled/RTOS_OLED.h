#include "../../DriverExport.h"




#ifndef OLEDDriver_H
#define OLEDDriver_H
//-----------------------------------------------------------
#define AWRN  BIT0  //R/W#
#define AA0   BIT1  //D/C#
#define RST   BIT2  //RESET#
#define ACSB  BIT3  //CS#
#define OLED_EN  BIT4  //OLED_ENABLE P5.4
#define OLEDDataBus PORT4
#define OLEDControlBus PORT5







extern OLEDCHART OLC;
extern void FullScreen();
extern void ClearScreen();
extern void Draw_Screen(unsigned char *Map);
extern void initial_ssd1303(void);
extern void Init_OLED(void);
extern void Dis_OLED();
extern void En_OLED();
extern void set_col(void);
extern void show_pic();
extern void SetXY(unsigned char X,unsigned char Y,unsigned char input)	;
extern unsigned char GetXY(unsigned char X,unsigned char Y);
extern void DrawLine(unsigned char StartX,unsigned char StartY,unsigned char EndX,unsigned char EndY);
extern void DrawBlock_Area(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char *BlockData);
extern void Full_Block(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char BlockData);
extern void DrawBlock(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char *BlockData);
extern void ClearBlock(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height);
extern void CreateChart(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned int MaxX,unsigned int MaxY,OLEDCHART *OLC);
extern void Print(unsigned char *CharData,unsigned char len,unsigned char Left,unsigned char Top,unsigned char fontsize);
extern void AddValue(OLEDCHART *OLC,int value);


//-----------------------------------------------------------
#endif
