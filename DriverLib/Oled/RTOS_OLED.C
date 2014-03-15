#include "RTOS_OLED.h"
#include "../PortManger.h"
#include "math.h"

unsigned char DC[1024]={0};
unsigned char RefreshPage[9]={0};
OLEDCHART OLC;

const char initial[16]=
{
 0xE2,   // Soft reset
 0xAE,   // Turn off oled
 0xAD,   // Set DC/DC
 0x8A,   // DC/DC disable
 0xA0,   // Segment re-map
 0xC8,   // Con re-map
 0xA8,   // Mutiplex
 0x3F,   // a[5:0]  : 0x40 = 64 display 64 column
 0x81,   // Contrast 0x81
 0x80,   // default 0x80(0X00~0XFF)
 0xD5,   // Display clock 0xD5
 0x10,   //
 0xDB,   // Power setting
 0xFF,
 0xA4,   // A5:entire Display (will turn on all pixel), A4:normal display (By data formate)
 0xAF    // Turn on oled
};

const char scroll[6]=
{
 0x2F,   // Stop horizontal scrolling 2F:Stop , 2E:Start
 0x25,   // Scrolling direction form right -> left
 0x02,   // Scrolling Step
 0x05,   // start page
 0x05,   // end page
 0x2E,   // Start horizontal scrolling
};


const unsigned char OLED_Size8_Dig[12][8]={
    {0x7E,0xFF,0x81,0x81,0xFF,0xFF,0x7E,0x00},   //0
    {0x00,0x82,0xFF,0xFF,0xFF,0x80,0x00,0x00},   //1
    {0x82,0xC3,0xE1,0xB9,0xBF,0x9F,0x8E,0x00},   //2
    {0x42,0xC3,0x99,0x99,0xFF,0xFF,0x6E,0x00},   //3
    {0x38,0x3C,0x2E,0x23,0x23,0xFF,0xFF,0x20},   //4
    {0x4F,0xCF,0x8D,0x8D,0xFD,0xFD,0x79,0x00},   //5
    {0x7E,0xFF,0x99,0x99,0xFB,0xFB,0x62,0x00},   //6
    {0x01,0x01,0xC1,0xF9,0xFF,0x3F,0x0F,0x00},   //7
    {0x6E,0xFF,0x99,0x99,0xFF,0xFF,0x6E,0x00},   //8
    {0x4E,0xDF,0x99,0x99,0xFF,0xFF,0x7E,0x00},   //9
    {0x00,0x00,0x00,0xC3,0xC3,0x00,0x00,0x00},   //:
    {0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x00},   //.
};
const unsigned char OLED_Size8_UpperWord[27][8]={
    {0xE0,0xFC,0x3F,0x23,0x3F,0x3F,0xFC,0xE0},   //A
    {0xFF,0xFF,0x99,0x99,0x99,0x99,0xFF,0x6E},   //B
    {0x7E,0xFF,0x81,0x81,0x81,0x81,0x81,0x00},   //C
    {0xFF,0xFF,0x81,0x81,0xC3,0xC3,0x7E,0x3C},   //D
    {0xFF,0xFF,0x99,0x99,0x99,0x99,0x81,0x00},   //E
    {0xFF,0xFF,0x19,0x19,0x19,0x19,0x01,0x00},   //F
    {0x7E,0xFF,0x81,0x81,0x99,0x99,0xF9,0x79},   //G
    {0xFF,0xFF,0x18,0x18,0x18,0x18,0xFF,0xFF},   //H
    {0x00,0x81,0xFF,0xFF,0xFF,0x81,0x00,0x00},   //I
    {0x60,0xE0,0x80,0x80,0xFF,0xFF,0x7F,0x00},   //J
    {0xFF,0xFF,0x18,0x3C,0x7E,0x6E,0xC3,0x81},   //K
    {0xFF,0xFF,0x80,0x80,0x80,0x80,0x80,0x00},   //L
    {0xFF,0xFF,0x0E,0x38,0x3E,0x0E,0xFF,0xFF},   //M
    {0xFF,0xFF,0x0C,0x18,0x38,0x20,0xFF,0xFF},   //N
    {0x7E,0xFF,0x81,0x81,0x81,0x81,0xFF,0x7E},   //O
    {0xFF,0xFF,0x19,0x19,0x19,0x19,0x1F,0x0E},   //P
    {0x7E,0xFF,0x81,0xC1,0xC1,0xC1,0xFF,0x7E},   //Q
    {0xFF,0xFF,0x19,0x39,0x79,0x79,0xDF,0x8E},   //R
    {0x4E,0xDF,0x99,0x99,0x99,0x99,0xFB,0x62},   //S
    {0x01,0x01,0xFF,0xFF,0xFF,0x01,0x01,0x00},   //T
    {0x7F,0xFF,0x80,0x80,0x80,0x80,0xFF,0x7F},   //U
    {0x0F,0x3F,0xF8,0xC0,0xF8,0xF8,0x3F,0x0F},   //V
    {0x7F,0xFF,0xC0,0x7C,0xFC,0xC0,0xFF,0x7F},   //W
    {0xE0,0xFC,0x3F,0x23,0x3F,0x3F,0xFC,0xE0},   //X   <---non
    {0x0F,0x1F,0xF8,0xF8,0xFF,0x1F,0x0F,0x00},   //Y
    {0xC1,0xE1,0xB9,0x9D,0x9F,0x8F,0x83,0x00},   //Z
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}    //Space or Other
    //{0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x00} 
};

float floatabs(float x){
  return (x < 0) ? -x : x;     
}
/*
int ceil(float x){
  return (int)x + ( x>0 && x!=(int)x );
}
*/
void OLED_Reset()		//OLED Reset
{
  P5OUT &= ~RST;
  P5OUT |= RST;			
}
void initial_ssd1303(void) 	//OLED初始化
 {
  unsigned char k;
  P4DIR=0xFF;
  P5DIR=0xFF;
  P5OUT = 0x41;  		// 0100 0001  set BS1、BS2、RD to Hi
  OLED_Reset();
  P5OUT &= ~(ACSB|AA0);
  //P5OUT &= ~AA0; 		
  //P5OUT &= ~ACSB; 		// set ACSB(CS#) to low
  for(k=0;k<16;k++)
  {     	
    P5OUT &= ~AWRN; 		//set AWRN to low	
    P4OUT = initial[k];
    P5OUT |= AWRN; 		
  }
}


void Init_OLED(void){
  initial_ssd1303();
  ClearScreen();						//clear data buffer
}

void Dis_OLED(){
  //P5OUT &= ~RST;
  //P5OUT &= ~OLED_EN;
  P5OUT &= ~(RST|OLED_EN);
}

void En_OLED(){
  P5OUT&=~OLED_EN;
  OLED_Reset();
  initial_ssd1303();
}

/*
void set_col(void){
  P5OUT &= ~AWRN;
    P4OUT=0x00;    		// Lower column
  P5OUT |= AWRN;

  P5OUT &= ~AWRN;
    P4OUT=0x10;   		// high column 0x60=96 , 0x1F=132
  P5OUT |= AWRN;
}
*/

void show_pic() {
  unsigned char page=0xB0;
  for(int i=0;i<1024;i+=128)  {
    if (RefreshPage[i>>7]==1){
      P5OUT &= ~AA0;
      
      P5OUT &= ~AWRN;	
        P4OUT=page;           	//page number Setting
      P5OUT |= AWRN;
      P5OUT &= ~AWRN;
        P4OUT=0x00;    		// Lower column
      P5OUT |= AWRN;
      P5OUT &= ~AWRN;
        P4OUT=0x10;   		// high column 0x60=96 , 0x1F=132
      P5OUT |= AWRN;
      
      P5OUT |= AA0;
      
      unsigned char *OLEDPT=&DC[i];
      for(int j=0;j<128;j++){	
        P5OUT &= ~AWRN;
        P4OUT=*OLEDPT++;
        P5OUT |= AWRN;	 	
      }
      RefreshPage[i>>7]=0;
    }
    page++;
  }
}


void ShowPIC1Page(){
  unsigned char page=0xB0;
  unsigned char Showed=0; //Show完一個PAGE就離開
  for(int i=0;i<8;i++)  {
    if(Showed==1)return;
    //----------------------------------------------------------------------
    if (RefreshPage[RefreshPage[8]]==1){
      P5OUT &= ~AA0;
      
      P5OUT &= ~AWRN;	
        P4OUT=page+RefreshPage[8];  //page number Setting
      P5OUT |= AWRN;
      P5OUT &= ~AWRN;
        P4OUT=0x00;    		// Lower column
      P5OUT |= AWRN;
      P5OUT &= ~AWRN;
        P4OUT=0x10;   		// high column 0x60=96 , 0x1F=132
      P5OUT |= AWRN;
      
      P5OUT |= AA0;
      
      unsigned char *OLEDPT=&DC[RefreshPage[8]*128];
      for(int j=0;j<128;j++){	
        P5OUT &= ~AWRN;
          P4OUT=*OLEDPT++;
        P5OUT |= AWRN;	 	
      }
      RefreshPage[RefreshPage[8]]=0;
      Showed=1;
    }
    //----------------------------------------------------------------------
    RefreshPage[8]++;
    RefreshPage[8]%=8;
  }
}


//-------------------------------------------------DRIVER LEVEL----------------------------------
void FullScreen(){
    for(int i=0;i<8;i++)RefreshPage[i]=1;
    for(int i=0;i<1024;i++)    
      DC[i]=0xFF;
}

void ClearScreen(){
    for(int i=0;i<8;i++)RefreshPage[i]=1;
    for(int i=0;i<1024;i++)      
      DC[i]=0x00;
}

void Draw_Screen(unsigned char *Map){
    for(int i=0;i<8;i++)RefreshPage[i]=1;
    for(int i=0;i<1024;i++){
      DC[i]=Map[i];
    }
}

void SetXY(unsigned char X,unsigned char Y,unsigned char input)		
{
    unsigned char Yled;
    switch ( Y & 7 )			
    { 					//Y mod 8(bits)
      case 0: Yled=BIT0; break;
      case 1: Yled=BIT1; break;
      case 2: Yled=BIT2; break;
      case 3: Yled=BIT3; break;
      case 4: Yled=BIT4; break;
      case 5: Yled=BIT5; break;
      case 6: Yled=BIT6; break;
      case 7: Yled=BIT7; break;		
    }
    unsigned int Row=(Y<<4)&0xFF8F;   		//Row=(Y/8)*128
    DC[Row+X]= (input==0)? (DC[Row+X] & ~Yled) : (DC[Row+X] | Yled);
    RefreshPage[Y>>3]=1;
}

///////////////////////////////////////////////////////////////////////////////////////////
unsigned char GetXY(unsigned char X,unsigned char Y)
{   //回傳一個X,Y是0 or 1
    unsigned char Yled;
    switch (Y&0x07){ //Y mod 8
      case 0: Yled=BIT0; break;
      case 1: Yled=BIT1; break;
      case 2: Yled=BIT2; break;
      case 3: Yled=BIT3; break;
      case 4: Yled=BIT4; break;
      case 5: Yled=BIT5; break;
      case 6: Yled=BIT6; break;
      case 7: Yled=BIT7; break;
    }
    unsigned int Row=(Y<<4)&0xFFF0;    //Row=Y/8*128=Y*16
    return  (DC[Row+X] & Yled)==0?0:1;
}

//-------------------------------------------------API LEVEL----------------------------------

void DrawLine(unsigned char StartX,unsigned char StartY,unsigned char EndX,unsigned char EndY)
{
    short w=EndX-StartX;
    short h=EndY-StartY;
    float m=0;
    if (fabs(w)>fabs(h)){
      	if (w!=0)
      	    m = (float)h / w;
      	if (w>0){
            for(int i = 0 ; i <= w ; i++)
              SetXY(StartX+i,(int)((float)StartY+i*m),1);
      	}else{
            for(int i = -w ; i >= 0 ; i--)
   	      SetXY(StartX-i,(int)((float)StartY-i*m),1);
      	}
    }else{
      if (h!=0)
      	m = (float)w / h;
      if (h>0){
          for(int i = 0 ; i <= h ; i++)
            SetXY((int)((float)StartX+i*m),StartY+i,1);	
      }else{
          for(int i = -h ; i >= 0 ; i--)
            SetXY((int)((float)StartX-i*m),StartY-i,1);
      }
    }
}

void DrawBlock_Area(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char *BlockData)  //將一BUFFER擴大填滿一個SIZE
{
  unsigned char ColData;  //0~256 column data
  for(int i=0;i<ceil(Height/8);i++){
    for(int j=0;j<Width;j++){
      ColData=BlockData[i*Width+j];
      for (int k=0;k<8;k++){
        SetXY(Left+j,Top+(i<<3)+k,ColData&0x01);
        ColData>>=1;
      }
    }
  }
}

void Full_Block(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char BlockData) //把整個高度為8的倍數的BLOCK填滿某一個固定的BYTE
{
  unsigned char ColData;  //0~256 column data
  for(int i=0;i<ceil(Height/8);i++){
    for(int j=0;j<Width;j++){
      ColData=BlockData;
      for (int k=0;k<8;k++){
        SetXY(Left+j,Top+(i<<3)+k,ColData&0x01);
        ColData>>=1;
      }
    }
  }
}


void DrawBlock(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned char *BlockData) //把一BUFFER擴大填滿一個高度為8的倍數的BLOCK
{
  unsigned char ColData;  //0~256 column data
  unsigned char rate=Height/8;
    for(int i=0;i<ceil(Height/Height);i++){
      for(int j=0;j<Width;j+=rate){
        ColData=BlockData[i*(Width/rate)+(j/rate)];
        for (int k=0;k<Height;k+=rate){          	
          for(int l=0;l<rate;l++){
            for(int m=0;m<rate;m++){
              SetXY(Left+j+l,Top+(i<<3)+k+m,ColData&0x01);
       	    }
          }
          ColData>>=1;
        }
      }	
    }

}

void ClearBlock(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height){
  Full_Block(Left,Top,Width,Height,0);
}

void CreateChart(unsigned char Left,unsigned char Top,unsigned char Width,unsigned char Height,unsigned int MaxX,unsigned int MaxY,OLEDCHART *OLC){
  ClearScreen();
  DrawLine(Left,Top,Left+Width,Top);                          //-----
  DrawLine(Left,Top,Left,Top+Height);                         //|
  DrawLine(Left,Top+Height,Left+Width,Top+Height);            //-----
  DrawLine(Left+Width,Top,Left+Width,Top+Height);             //|
  OLC->Left=Left+1;
  OLC->Top=Top+1;
  OLC->Width=Width-1;
  OLC->Height=Height-1;
  OLC->MaxX=MaxX;
  OLC->MaxY=MaxY;
  OLC->Offset= MaxX/Width;
  OLC->NowX=0;
  OLC->NowCount=0;
  OLC->OrgAxis = (OLC->Top + (OLC->Height >> 1));
}

void Print(unsigned char *CharData,unsigned char len,unsigned char Left,unsigned char Top,unsigned char fontsize){
  //Full_Block(Left,Top,len*(fontsize+1),fontsize+1,0x00);
  for(int i=0;i<len;i++){
    if ((CharData[i]<0x3A)&&(CharData[i]>=0x30)) //顯示0~9(ASCILL)
      DrawBlock(Left+(i*fontsize),Top,fontsize,fontsize,(unsigned char *)OLED_Size8_Dig[CharData[i]-0x30]);
    else if ((CharData[i]>0x40)&&(CharData[i]<0x5B)) //顯示A~Z
      DrawBlock(Left+(i*fontsize),Top,fontsize,fontsize,(unsigned char *)OLED_Size8_UpperWord[CharData[i]-0x41]);
    else if(CharData[i]==':')
      DrawBlock(Left+(i*fontsize),Top,fontsize,fontsize,(unsigned char *)OLED_Size8_Dig[10]);
    else if(CharData[i]=='.')
      DrawBlock(Left+(i*fontsize),Top,fontsize,fontsize,(unsigned char *)OLED_Size8_Dig[11]);
    else if(CharData[i]==' ')
      Full_Block(Left+(i*fontsize),Top,fontsize,fontsize,0x00);
    if(Left+((i+2)*fontsize)<128)Full_Block(Left+((i+1)*fontsize),Top,fontsize,fontsize,0x00);
    if (CharData[i]==0x00)break;
  }
  //第一個字再重印一次
}


void AddValue(OLEDCHART *OLC,int value){
    OLC->NowX++;
    if (OLC->NowX==OLC->Offset){   //draw value when NowX is euqal to Offset

        //if(abs(value) < OLC->MaxY) Val=value/((float)OLC->MaxY/(float)OLC->Height);
        int Val = (int)((float)value /((float)OLC->MaxY/(float)OLC->Height));
        for (int i=0;i<OLC->Height;i++) SetXY(OLC->Left+OLC->NowCount,OLC->Top+i,0);  //clear column        

        DrawLine(OLC->Left+OLC->NowCount-1,
                 OLC->OrgAxis+OLC->PreVal,
                 OLC->Left+OLC->NowCount,
                 OLC->OrgAxis+Val);

/*
        DrawLine(OLC->Left+(OLC->NowCount%OLC->Width)-1,
                 OLC->OrgAxis + OLC->PreVal,
                 OLC->Left+(OLC->NowCount%OLC->Width),
                 OLC->OrgAxis - Val); 
*/
        //SetXY(OLC->Left+OLC->NowCount,OLC->Top+Val,1);  //draw point
        OLC->NowCount++;
        OLC->NowCount%=OLC->Width;
        OLC->PreVal=Val;
    }
    OLC->NowX %= OLC->Offset;
}






/***********************************使用GPIO函式庫..比較慢********************************

void OLED_Reset(){		//OLED Reset
  OutputGPIOLo(OLEDControlBus|RST);
  OutputGPIOHi(OLEDControlBus|RST);
}

void initial_ssd1303(void){ 	//OLED初始化
  unsigned char k;
  SetGPIOOutput(OLEDDataBus|0xff);
  SetGPIOOutput(OLEDControlBus|0xff);
  OutputGPIOHi(OLEDControlBus|OLED_EN|AWRN);
  OLED_Reset();
  OutputGPIOLo(OLEDControlBus|AA0|ACSB);
  for(k=0;k<16;k++)  {   
    OutputGPIO(OLEDDataBus|initial[k]);
    OutputGPIOLo(OLEDControlBus|AWRN);//set AWRN to low
    OutputGPIOHi(OLEDControlBus|AWRN);
  }
}


void Init_OLED(void)
{
   //////// for OLED use ////////
    SetGPIOOutput(OLEDDataBus|0xff);
    SetGPIOOutput(OLEDDataBus|0xff);
    OutputGPIOHi(OLEDControlBus|AWRN|OLED_EN);  // 00010001 set OLED_ENABLE,OLED_R/W#
   /////////////////////////
   //int time,a,x,i,j=0;
  	initial_ssd1303();
  	//CreateChart(0,16,127,47,128,256,&OLC);			//creat picture boundary
  	//for (u32 k=0;k<40000;k++);				//for delay
  	ClearScreen();						//clear data buffer
  	//show_pic(); 						//clear picture
}

void Dis_OLED(){
  OutputGPIOLo(OLEDControlBus|RST|OLED_EN);
  //OLEDIsLight=0;
}

void En_OLED(){
  OutputGPIOLo(OLEDControlBus|OLED_EN);
  OLED_Reset();
  initial_ssd1303();
  //OLEDIsLight=1;
}


void set_col(void)
{
  OutputGPIO(OLEDDataBus|0x00);  // Lower column
  OutputGPIOLo(OLEDControlBus|AWRN);
  OutputGPIOHi(OLEDControlBus|AWRN);
  OutputGPIO(OLEDDataBus|0x10);  // high column 0x60=96 , 0x1F=132
  OutputGPIOLo(OLEDControlBus|AWRN);
  OutputGPIOHi(OLEDControlBus|AWRN);
}

void show_pic() {
  unsigned char page;//,Unsusses;
  unsigned int i,j;
  page=0xB0;  		
  for(i=0;i<1024;i+=128)  {
    if (RefreshPage[i>>7]==1){
      OutputGPIOLo(OLEDControlBus|AA0);
      OutputGPIO(OLEDDataBus|page);  //page number Setting
      OutputGPIOLo(OLEDControlBus|AWRN);
      OutputGPIOHi(OLEDControlBus|AWRN);
      set_col();             	// setup low/high column
      OutputGPIOHi(OLEDControlBus|AA0);
      for(j=0;j<128;j++){	
        OutputGPIO(OLEDDataBus|DC[j+i]);
        OutputGPIOLo(OLEDControlBus|AWRN);
        OutputGPIOHi(OLEDControlBus|AWRN);        	
      }
      RefreshPage[i>>7]=0;
    }
    page++;
  }
}

***********************************************************************************/

