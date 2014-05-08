#include "LCD_ili9341.h"
#include "MSP430F5438a.h"
#include "stdlib.h"
#include "INTRINSICS.H"
#include "font.h"

#define CPU_F ((double)16000000)
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

typedef unsigned short     int u16;
typedef unsigned          char u8;
typedef unsigned           int u32;

#define FONT_SIZE_16 16
#define FONT_SIZE_12 12

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


u16 g_background=BLACK;
u16 g_forground=WHITE;

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

void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
	LCD_Scan_Dir(L2R_U2D);
        
	xlen=ex-sx+1;	   
	for(i=sy;i<=ey;i++)
	{
	 	LCD_SetCursor(sx,i);
		LCD_WriteRAM_Prepare();  
		for(j=0;j<xlen;j++)
            LCD_WriteData(color);
	}

	LCD_Scan_Dir(DFT_SCAN_DIR);
}  

void LCD_DisplayOn(void)
{
    LCD_WriteReg(0x29);    //Display on 
    
	LCD_Scan_Dir(DFT_SCAN_DIR);		 
	LCD_LED_SET;
	LCD_Clear(g_background);    
}

void LCD_ClearPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);
	LCD_WriteRAM_Prepare();
	LCD_WriteData(g_background); 
}

void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);
	LCD_WriteRAM_Prepare();
	LCD_WriteData(g_forground); 
}

void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1;
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1;
	else if(delta_x==0)incx=0;
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x;
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )
	{  
		LCD_DrawPoint(uRow,uCol);
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   

void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2 )
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}

void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3           
		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0-a,y0+b);             //1       
		LCD_DrawPoint(x0-b,y0-a);             //7           
		LCD_DrawPoint(x0-a,y0-b);             //2             
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-b,y0+a);             
		a++;   
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b);
	}
} 

void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{     
  #define MAX_CHAR_POSX 232
  #define MAX_CHAR_POSY 312

    u8 temp;
    u8 pos,t;
	u16 x0=x;   
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY)return;	    
   
	num=num-' ';
	if(!mode) 
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];
			else temp=asc2_1608[num][pos];
			for(t=0;t<size/2;t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint(x,y);	
				else LCD_ClearPoint(x,y);
				
				temp>>=1; 
				x++;
		    }
			x=x0;
			y++;
		}	
	}else
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];
			else temp=asc2_1608[num][pos];		 
			for(t=0;t<size/2;t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);
		        temp>>=1; 
		    }
		}
	}	   	 	  
} 

u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}	


void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 

void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode)
{         	
	u8 t,temp;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode); 
	}
} 


void LCD_ShowString(u16 x,u16 y,const u8 *p,u8 size)
{         
    while(*p!='\0')
    {       
        if(x>MAX_CHAR_POSX){x=0;y+=16;}
        if(y>MAX_CHAR_POSY){y=x=0;LCD_Clear(g_background);}
        LCD_ShowChar(x,y,*p,size,0);
        x+=8;
        p++;
    }  
}

void LCD_ShowfloatNum(u16 x,u16 y,float num,u8 lenZ,u8 lenX,u8 size)
{
      unsigned char i;
      float numX;
      numX = num - (unsigned int)num;     
      for(i=0;i<lenX;i++)
        numX = numX * 10;
      LCD_ShowNum(x,y,(unsigned int)num,lenZ,size);
      if(size == 12)
      {
        LCD_ShowChar(x+lenZ*6,y,'.',size,0);
        LCD_ShowNum(x+(lenZ+1)*6,y,(unsigned int)numX,lenX,size);
      }else
      {
        LCD_ShowChar(x+lenZ*8,y,'.',size,0);
        LCD_ShowNum(x+(lenZ+1)*8,y,(unsigned int)numX,lenX,size);
      
      }  
}

void LCD_SetColor( u16 forground , u16 background )
{
    g_forground = forground;
    g_background = background;
}

void LCD_Init(void)
{
    LCD_SetColor( WHITE , BLACK );
    
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

    Draw_Circle(100 , 200 , 30);

    LCD_DrawRectangle( 120 , 220 , 150 , 270 );

    int line_number=0;
    LCD_ShowString(0,line_number++,"abcdefghijklmnopqrstuvwxyz",FONT_SIZE_12);	
    LCD_ShowString(0,FONT_SIZE_12*line_number++,"0123456789",FONT_SIZE_12);	    

    LCD_ShowString(0,FONT_SIZE_12*line_number++,"abcdefghijklmnopqrstuvwxyz",FONT_SIZE_16);	
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"0123456789",FONT_SIZE_16);	    
    
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"+-----------------------+",FONT_SIZE_16);
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"|  |   |          |     |",FONT_SIZE_16);
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"|  |---|,---.. . .|__/  |",FONT_SIZE_16);
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"|  |   |,---|| | ||  \\  |",FONT_SIZE_16);
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"|  `   '`---^`-'-'`   ` |",FONT_SIZE_16);
    LCD_ShowString(0,FONT_SIZE_16*line_number++,"+-----------------------+",FONT_SIZE_16);
                            
 //   LCD_ShowNum(30,110,53345405,6,16);
 //  LCD_ShowfloatNum(30,130,3.14159265,7,4,16);
  
}