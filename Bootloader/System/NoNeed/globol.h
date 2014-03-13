//#define f449
//#define f449
//#define f169
//#include  "msp430x14x.h"

#include  <msp430x16x.h>
/*

#define BYTE unsigned char
#define UINT unsigned int
#define ULONG unsigned long

typedef unsigned char        	ubyte;		//位元
typedef signed char          	byte;
typedef unsigned short       	uword;		//字元
typedef signed short         	word;
typedef unsigned long int      	udword;		//雙字元
typedef signed long int        	dword;	
*/
#define BOOL char
#define SCHAR char
#define u8 unsigned char 
#define u16 unsigned int 
#define u32 unsigned long


#define s8 signed char 
#define s16 signed int 
#define s32 signed 

#define Crystal 32768

#define NULL 0

#define ECG_SAMPLE_OK   1
#define RTC_1S          2

#define BIT10   (1<<9)
#define BIT11   (1<<10)
#define BIT12   (1<<11)
#define BIT13   (1<<12)
#define BIT14   (1<<13)
#define BIT15   (1<<14)

#define USER_ID_LEN 4

