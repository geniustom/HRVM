#define MSP430_5x
//#define MSP430_16x

#include "MSP430.h"

#ifdef MSP430_5x

/************Interrupt************/
#define IE1             SFRIE1
#define IFG1            SFRIFG1
/************  TimerA ************/
#define TACCR0          TA0CCR0
#define TACCR1          TA0CCR1
#define TACCR2          TA0CCR2
#define TACCTL0         TA0CCTL0
#define TACCTL1         TA0CCTL1
#define TACCTL2         TA0CCTL2
#define TAIV            TA0IV
#define TAR             TA0R
#define TACTL           TA0CTL
/************   ADC  ************/
#define SHT0_1          ADC12SHT0_1
#define REF2_5V         ADC12REF2_5V
#define MSC             ADC12MSC
#define CONSEQ_3        ADC12CONSEQ_3
#define SHS_0           ADC12SHS_0
#define SHP             ADC12SHP
#define INCH_0          ADC12INCH_0
#define SREF_1          ADC12SREF_1
#define INCH_1          ADC12INCH_1
#define INCH_2          ADC12INCH_2
#define EOS             ADC12EOS
#define ENC             ADC12ENC
/************   ISR  ************/
#define TIMERA0_VECTOR  TIMER0_A0_VECTOR
#define TIMERA1_VECTOR  TIMER0_A1_VECTOR
#define UART0RX_VECTOR  USCI_A1_VECTOR
#define ADC_VECTOR      ADC12_VECTOR

#endif
