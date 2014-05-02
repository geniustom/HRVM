#include "LCD_ili9341.h"
#include "MSP430F5438a.h"
#include "stdlib.h"
#include "INTRINSICS.H"


#define CPU_F ((double)16000000)
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))


typedef unsigned short     int u16;
typedef unsigned          char u8;
typedef unsigned           int u32;


#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430
#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C
#define GRAYBLUE       	 0X5458
#define LIGHTGREEN     	 0X841F
#define LIGHTGRAY        0XEF5B
#define LGRAY 			 0XC618
#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12


#define	LCD_LED_SET     P3OUT |= BIT6
#define	LCD_RST_SET     P3OUT |= BIT3
#define	LCD_CS_SET      P3OUT |= BIT2
#define	LCD_RS_SET	    P3OUT |= BIT0
#define	LCD_WR_SET	    P3OUT |= BIT1
#define	LCD_RD_SET	    P3OUT |= BIT7

#define	LCD_RST_CLR     P3OUT &= ~BIT3
#define	LCD_CS_CLR      P3OUT &= ~BIT2
#define	LCD_RS_CLR      P3OUT &= ~BIT0
#define	LCD_WR_CLR	    P3OUT &= ~BIT1
#define	LCD_RD_CLR	    P3OUT &= ~BIT7

#define L2R_U2D  0
#define L2R_D2U  1
#define R2L_U2D  2
#define R2L_D2U  3

#define U2D_L2R  4
#define U2D_R2L  5
#define D2U_L2R  6
#define D2U_R2L  7

#define DFT_SCAN_DIR  L2R_U2D

void LCD_SetIOOutput()
{
    P9DIR = 0xFF;
    P4DIR = 0xFF;
}

void LCD_WriteReg(u8 data)
{ 
    LCD_RD_SET;     
    LCD_RS_CLR;
    LCD_CS_CLR;  

    P4OUT = data;
    
    LCD_WR_CLR; 
    LCD_WR_SET; 
    LCD_CS_SET;   
}

void LCD_WriteData(u16 data)
{
    LCD_RD_SET;  
    LCD_RS_SET;
    LCD_CS_CLR;
    LCD_WR_CLR;
    
    P9OUT = data>>8;
    P4OUT = data&0xff;
    
    LCD_WR_SET;
    LCD_CS_SET;
}

void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	    
    LCD_WriteReg(0x2A); 
    LCD_WriteData(Xpos>>8); 
    LCD_WriteData(Xpos&0XFF);	 
    LCD_WriteReg(0x2B); 
    LCD_WriteData(Ypos>>8); 
    LCD_WriteData(Ypos&0XFF);						    
}

void LCD_WriteRAM_Prepare(void)
{
	LCD_WriteReg(0x2C);
}	

void LCD_Clear(u16 Color)
{
	u16 index=0,indey=0;      
	LCD_SetCursor(0x00,0x0000);
	LCD_WriteRAM_Prepare();	  
	for(indey=0;indey<320;indey++)
	{
        for(index=0;index<240;index++)
            LCD_WriteData(Color);    
	}
}  

void LCD_Scan_Dir(u8 dir)
{
    u16 regval=0;
    u8 dirreg=0;
    switch(dir)
    {
    case L2R_U2D:
        regval|=(0<<7)|(0<<6)|(0<<5); 
        break;
    case L2R_D2U:
        regval|=(1<<7)|(0<<6)|(0<<5); 
        break;
    case R2L_U2D:
        regval|=(0<<7)|(1<<6)|(0<<5); 
        break;
    case R2L_D2U:
        regval|=(1<<7)|(1<<6)|(0<<5); 
        break;	 
    case U2D_L2R:
        regval|=(0<<7)|(0<<6)|(1<<5); 
        break;
    case U2D_R2L:
        regval|=(0<<7)|(1<<6)|(1<<5); 
        break;
    case D2U_L2R:
        regval|=(1<<7)|(0<<6)|(1<<5); 
        break;
    case D2U_R2L:
        regval|=(1<<7)|(1<<6)|(1<<5); 
        break;	 
    }
    dirreg=0X36;
    regval|=0X08;//BGR   
    LCD_WriteReg(dirreg);  
    LCD_WriteData(regval);	  
    
    if(regval&0X20)
    {
        LCD_WriteReg(0x2A); 
        LCD_WriteData(0);
        LCD_WriteData(0);
        LCD_WriteData(319>>8);
        LCD_WriteData(319&0XFF);
        LCD_WriteReg(0x2B); 
        LCD_WriteData(0);
        LCD_WriteData(0);
        LCD_WriteData(239>>8);
        LCD_WriteData(239&0XFF);
    }
    else 
    {
        LCD_WriteReg(0x2A); 
        LCD_WriteData(0);
        LCD_WriteData(0);
        LCD_WriteData(239>>8);
        LCD_WriteData(239&0XFF);
        LCD_WriteReg(0x2B); 
        LCD_WriteData(0);
        LCD_WriteData(0);
        LCD_WriteData(319>>8);
        LCD_WriteData(319&0XFF);
    }  
} 

void LCD_DisplayOn(void)
{
    LCD_WriteReg(0x29);    //Display on 
    
	LCD_Scan_Dir(DFT_SCAN_DIR);		 
	LCD_LED_SET;
	LCD_Clear(DARKBLUE);    
}

void LCD_Init(void)
{
    P3DIR = 0xff;
    P3OUT = 0xff;  
    P4DIR = 0xff;
    P4OUT = 0xff;
    P9DIR = 0xff;
    P9OUT = 0xff;
    P8DIR = BIT3;
    P8OUT = BIT3;

    LCD_SetIOOutput();
    LCD_WriteReg(0xCF);  
    LCD_WriteData(0x00); 
    LCD_WriteData(0xC1); 
    LCD_WriteData(0X30); 
    LCD_WriteReg(0xED);  
    LCD_WriteData(0x64); 
    LCD_WriteData(0x03); 
    LCD_WriteData(0X12); 
    LCD_WriteData(0X81); 
    LCD_WriteReg(0xE8);  
    LCD_WriteData(0x85); 
    LCD_WriteData(0x10); 
    LCD_WriteData(0x7A); 
    LCD_WriteReg(0xCB);  
    LCD_WriteData(0x39); 
    LCD_WriteData(0x2C); 
    LCD_WriteData(0x00); 
    LCD_WriteData(0x34); 
    LCD_WriteData(0x02); 
    LCD_WriteReg(0xF7);  
    LCD_WriteData(0x20); 
    LCD_WriteReg(0xEA);  
    LCD_WriteData(0x00); 
    LCD_WriteData(0x00); 
    LCD_WriteReg(0xC0);    //Power control 
    LCD_WriteData(0x1B);   //VRH[5:0] 
    LCD_WriteReg(0xC1);    //Power control 
    LCD_WriteData(0x01);   //SAP[2:0];BT[3:0] 
    LCD_WriteReg(0xC5);    //VCM control 
    LCD_WriteData(0x30); 	 //3F
    LCD_WriteData(0x30); 	 //3C
    LCD_WriteReg(0xC7);    //VCM control2 
    LCD_WriteData(0XB7); 
    LCD_WriteReg(0x36);    // Memory Access Control 
    LCD_WriteData(0x48); 
    LCD_WriteReg(0x3A);   
    LCD_WriteData(0x55); 
    LCD_WriteReg(0xB1);   
    LCD_WriteData(0x00);   
    LCD_WriteData(0x1A); 
    LCD_WriteReg(0xB6);    // Display Function Control 
    LCD_WriteData(0x0A); 
    LCD_WriteData(0xA2); 
    LCD_WriteReg(0xF2);    // 3Gamma Function Disable 
    LCD_WriteData(0x00); 
    LCD_WriteReg(0x26);    //Gamma curve selected 
    LCD_WriteData(0x01); 
    LCD_WriteReg(0xE0);    //Set Gamma 
    LCD_WriteData(0x0F); 
    LCD_WriteData(0x2A); 
    LCD_WriteData(0x28); 
    LCD_WriteData(0x08); 
    LCD_WriteData(0x0E); 
    LCD_WriteData(0x08); 
    LCD_WriteData(0x54); 
    LCD_WriteData(0XA9); 
    LCD_WriteData(0x43); 
    LCD_WriteData(0x0A); 
    LCD_WriteData(0x0F); 
    LCD_WriteData(0x00); 
    LCD_WriteData(0x00); 
    LCD_WriteData(0x00); 
    LCD_WriteData(0x00); 		 
    LCD_WriteReg(0XE1);    //Set Gamma 
    LCD_WriteData(0x00); 
    LCD_WriteData(0x15); 
    LCD_WriteData(0x17); 
    LCD_WriteData(0x07); 
    LCD_WriteData(0x11); 
    LCD_WriteData(0x06); 
    LCD_WriteData(0x2B); 
    LCD_WriteData(0x56); 
    LCD_WriteData(0x3C); 
    LCD_WriteData(0x05); 
    LCD_WriteData(0x10); 
    LCD_WriteData(0x0F); 
    LCD_WriteData(0x3F); 
    LCD_WriteData(0x3F); 
    LCD_WriteData(0x0F); 
    LCD_WriteReg(0x2B); 
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x01);
    LCD_WriteData(0x3f);
    LCD_WriteReg(0x2A); 
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0xef);	 
    LCD_WriteReg(0x11); //Exit Sleep
}

void LCD_Test(void)
{
    LCD_Init();
    delay_ms(120); 
    LCD_DisplayOn();
}