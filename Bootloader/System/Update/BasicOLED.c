#include "OLED.h"
#include "globol.h"
#define abs(x) x>=0?x:-x

const char initial[18]=
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
 0xD5,   // Display clock
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

void OLED_Reset()		//OLED Reset
{
  P5OUT &= ~RST;
  P5OUT |= RST;			
}

void Dis_OLED(){
  P5OUT &= ~RST;
  P5OUT&=~OLED_EN;
}

void initial_ssd1303(void) 	//OLEDªì©l¤Æ
 {
  unsigned char k;
  P4DIR=0xFF;
  P5DIR=0xFF;
  P5OUT = 0x41;  		// 0100 0001  set BS1¡BBS2¡BRD to Hi
  OLED_Reset();
  P5OUT &= ~AA0; 		
  P5OUT &= ~ACSB; 		// set ACSB(CS#) to low
  for(k=0;k<16;k++)
  {     	
    P4OUT = initial[k];
    P5OUT &= ~AWRN; 		//set AWRN to low	
    P5OUT |= AWRN; 		
  }
}


void En_OLED(){
  P5OUT&=~OLED_EN;
  OLED_Reset();
  initial_ssd1303();
}

void set_col(void)
{
  P4OUT=0x00;    		// Lower column
  P5OUT &= ~AWRN;
  P5OUT |= AWRN;

  P4OUT=0x10;   		// high column 0x60=96 , 0x1F=132
  P5OUT &= ~AWRN;
  P5OUT |= AWRN;
}

void ClearScreen(){
  unsigned char page;
  u16 i,j;
  page=0xB0;  		
  for(i=0;i<1024;i+=128)  {
      P5OUT &= ~AA0;	  	
      P4OUT=page;           	//page number Setting
      P5OUT &= ~AWRN;	
      P5OUT |= AWRN;	
      set_col();             	// setup low/high column
      P5OUT |= AA0;
      for(j=0;j<128;j++){	
        //page_data=pic[pic_index][j+i*128];
        //page_data=DC[j+(i<<7)];
        P4OUT=0x00;
        //P4OUT=0X03;
        P5OUT &= ~AWRN;	
        P5OUT |= AWRN;	 	
      }
      page++;
  }
}

void ShowProgress(u8 progress){
  unsigned char page;
  u16 j;
  //ClearScreen();
  page=0xB0;  		
  P5OUT &= ~AA0;	  	
  P4OUT=page;           	//page number Setting
  P5OUT &= ~AWRN;	
  P5OUT |= AWRN;	
  set_col();             	// setup low/high column
  P5OUT |= AA0;
  for(j=0;j<128;j++){	
    //page_data=pic[pic_index][j+i*128];
    //page_data=DC[j+(i<<7)];
    if (j<=progress){
      P4OUT=0xFF;
    }else{
      P4OUT=0x00;
    };
    //P4OUT=0X03;
    P5OUT &= ~AWRN;	
    P5OUT |= AWRN;	 	
  }
}

void Init_OLED(void)
{
   //////// for OLED use ////////
  	P4DIR|=0xFF;				    		
  	P5DIR|=0xFF;    								
  	P5OUT = 0x41;  						// 01000001 set OLED_ENABLE,OLED_R/W#
   /////////////////////////
   //int time,a,x,i,j=0;
  	initial_ssd1303();
  	//CreateChart(0,16,127,47,128,256,&OLC);			//creat picture boundary
  	//for (u32 k=0;k<40000;k++);				//for delay

  	//show_pic(); 						//clear picture
}







/*
void show_pic() {
  unsigned char page,Unsusses;
  u16 i,j;
  page=0xB0;  		
  for(i=0;i<1024;i+=128)  {
    if (RefreshPage[i>>7]==1){
      P5OUT &= ~AA0;	  	
      P4OUT=page;           	//page number Setting
      P5OUT &= ~AWRN;	
      P5OUT |= AWRN;	
      set_col();             	// setup low/high column
      P5OUT |= AA0;
      for(j=0;j<128;j++){	
        //page_data=pic[pic_index][j+i*128];
        //page_data=DC[j+(i<<7)];
        P4OUT=DC[j+i];
        //P4OUT=0X03;
        P5OUT &= ~AWRN;	
        P5OUT |= AWRN;	 	
      }
      RefreshPage[i>>7]=0;
    }
    page++;
  }
*/

