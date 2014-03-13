#include "globol.h"
#include "MATH.h"
//#include "application/MAP.h"

#define AWRN  BIT0  //R/W#
#define AA0   BIT1  //D/C#
#define RST   BIT2  //RESET#
#define ACSB  BIT3  //CS#

#define OLED_EN  BIT6  //OLED_ENABLE P5.6

extern void Print(u8 *CharData,u8 len,u8 Left,u8 Top,u8 fontsize);
extern unsigned char OLEDIsLight;

typedef struct 
{
  u8 Top;
  u8 Left;
  u8 Width;
  u8 Height;
  u16 MaxX;
  u16 MaxY;
  u8 Offset;  //The downsampling
  u8 NowX;
  u8 NowCount;
  u8 PreVal;
  u8 OrgAxis;
  //u16 page;
  //u16 HP[8];
}
OLEDCHART;

/*
const unsigned char OLED_Size16_ChineseWord[6][16]={
    {0x00,0x00,0x20,0x20,0xFE,0x6A,0x6A,0x6A,0xEA,0x6A,0x6A,0xFE,0x20,0x20,0x00,0x00},
    {0x00,0x00,0x10,0x14,0x17,0x15,0x15,0x15,0x1F,0x15,0x15,0x17,0x14,0x10,0x00,0x00},//¶q
    {0x00,0x00,0x18,0x0A,0xDA,0x5A,0x4A,0xFE,0x4A,0x5A,0x5A,0xCA,0x18,0x08,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x07,0x05,0x05,0x1F,0x15,0x15,0x15,0x17,0x10,0x1C,0x00,0x00},//¹q
};
*/


void FullScreen();
void ClearScreen();
void Draw_Screen(unsigned char *Map);
void initial_ssd1303(void);
void Init_OLED(void);
void Dis_OLED();
void En_OLED();
void set_col(void);
void show_pic();
void SetXY(u8 X,u8 Y,u8 input)	;
u8 GetXY(u8 X,u8 Y);
void DrawLine(u8 StartX,u8 StartY,u8 EndX,u8 EndY);
void DrawBlock_Area(u8 Left,u8 Top,u8 Width,u8 Height,u8 *BlockData);
void Full_Block(u8 Left,u8 Top,u8 Width,u8 Height,u8 BlockData);
void DrawBlock(u8 Left,u8 Top,u8 Width,u8 Height,u8 *BlockData);
void ClearBlock(u8 Left,u8 Top,u8 Width,u8 Height);
void CreateChart(u8 Left,u8 Top,u8 Width,u8 Height,u16 MaxX,u16 MaxY,OLEDCHART *OLC);
void Print(u8 *CharData,u8 len,u8 Left,u8 Top,u8 fontsize);
void AddValue(OLEDCHART *OLC,int value);
