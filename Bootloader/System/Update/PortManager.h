#include "globol.h"

#define   P0_BIT0         BIT0
#define   P0_BIT1         BIT1
#define   P0_BIT2         BIT2
#define   P0_BIT3         BIT4
#define   P0_BIT4         BIT4
#define   P0_BIT5         BIT5
#define   P0_BIT6         BIT6
#define   P0_BIT7         BIT7

#define   P1_BIT0         BIT0
#define   P1_BIT1         BIT1
#define   P1_BIT2         BIT2
#define   P1_BIT3         BIT3
#define   P1_BIT4         BIT4
#define   P1_BIT5         BIT5
#define   P1_BIT6         BIT6
#define   P1_BIT7         BIT7

#define   P2_BIT0         BIT0
#define   P2_BIT1         BIT1
#define   P2_BIT2         BIT2
#define   P2_BIT3         BIT3
#define   P2_BIT4         BIT4
#define   P2_BIT5         BIT5
#define   P2_BIT6         BIT6
#define   P2_BIT7         BIT7

#define   P3_BIT0         BIT0
#define   P3_BIT1         BIT1
#define   P3_BIT2         BIT2
#define   P3_BIT3         BIT3
#define   P3_BIT4         BIT4
#define   P3_BIT5         BIT5
#define   P3_BIT6         BIT6
#define   P3_BIT7         BIT7

#define   P4_BIT0         BIT0
#define   P4_BIT1         BIT1
#define   P4_BIT2         BIT2
#define   P4_BIT3         BIT3
#define   P4_BIT4         BIT4
#define   P4_BIT5         BIT5
#define   P4_BIT6         BIT6
#define   P4_BIT7         BIT7

#define   P5_BIT0         BIT0
#define   P5_BIT1         BIT1
#define   P5_BIT2         BIT2
#define   P5_BIT3         BIT3
#define   P5_BIT4         BIT4
#define   P5_BIT5         BIT5
#define   P5_BIT6         BIT6
#define   P5_BIT7         BIT7

#define   P6_BIT0         BIT0
#define   P6_BIT1         BIT1
#define   P6_BIT2         BIT2
#define   P6_BIT3         BIT3
#define   P6_BIT4         BIT4
#define   P6_BIT5         BIT5
#define   P6_BIT6         BIT6
#define   P6_BIT7         BIT7


#define LOW       0
#define NEG_EDGE  0

#define POS_EDGE  1
#define HIGH      1

#define PUSH      POS_EDGE
#define FREE      LOW

#define P1  0
#define P2  1
#define P3  2
#define P4  3
#define P5  4
#define P6  5

#define UART0 (BIT4 | BIT5) //Port3
#define UART1 (BIT6 | BIT7) //Port3
#define SPI0 (BIT1 | BIT2 | BIT3) //Port 3
#define SPI1 (BIT1 | BIT2 | BIT3) //Port 5


//#define ACQUISITION {PIEZO | PEDO | VOICE | TEMP | ECG};

//Port1
#define BUTTONEnter            P2_BIT0
#define BUTTONLeft             P2_BIT1
#define BUTTONRight            P2_BIT2


/*============= NAMD FLASH CONTROL BUS INITIAL STATES =================*/ 
#define	  COMMAND_LATCH	  	0XFE//0X16
#define	  ADDRESS_LATCH   	0XFD//0X15
#define   INPUT_DATA_LATCH	0XF8//0X32//0X12
#define   OUTPUT_DATA_LATCH	0XF8//0X58	
#define   INSERT_ADDRESS_LATCH	0XFD//0X35//0X5A
#define   STATUS_READ_LATCH	0xF8//0X3C//0X1C

/*=============NAMD FLASH FUNCTION PIN OF MSP430F1612=================*/

#define	  CLE	          0XFE	//BIT0
#define   ALE	          0XFD	//BIT1
#define   CE	          0XFB  //BIT2
#define   RE	          0XF7  //BIT3
#define   WE	          0XBF  //BIT6
#define   WP	          0X7F  //BIT7
///// P2.7 /////
#define   RB	          0X7F  //BIT6
////////////////
#define	  CONTROL	  	  P3OUT
#define   DATA_OUT	  	  P1OUT
#define   DATA_IN	      P1IN


#define ECG         BIT2       //Port6
#define BATTERY     BIT1       //Port6
#define ECGShutdown P5_BIT4 | P5_BIT5 //P5.4,P5.5 ECGShutdown
#define BeepPort P6_BIT3;















/*============= I/O PORT FUNCTION SET =================*/
/*============= SD CARD PORT SET =================
#define   SD_DOUT         P3_BIT1
#define   SD_CLK          P3_BIT3
#define   SD_CS           P3_BIT0
#define   SD_VCC          P2_BIT7
#define   SD_INSERT       P3_BIT4
#define   SD_WRITING      P4_BIT7
*/
//#define BT_POWER P3_BIT5       //Port2
//#define BT_SW    BIT6       //Port2
//#define BT_ISCON P2_BIT1       //Port2
//#define BT BT_POWER | BT_SW | BT_ISCON;
//#define PEAD_DETECTION  P4_BIT6

//#define VOICE    BIT2       //Port6
//#define TEMP     BIT1       //Port6
//#define ECG      BIT0       //Port6
//#define PIEZO    BIT3       //Port6
//#define PEDO     BIT4       //Port6
