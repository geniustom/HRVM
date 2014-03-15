#include "../../DriverExport.h"

void (*COM0CallBack)(unsigned char RX);  //回呼的FUNCTION POINT
void (*COM1CallBack)(unsigned char RX);  //回呼的FUNCTION POINT
void (*COM2CallBack)(unsigned char RX);  //回呼的FUNCTION POINT
void (*COM3CallBack)(unsigned char RX);  //回呼的FUNCTION POINT

void SetBaudrate(unsigned long SYSCLK,unsigned long Baudrate,
                 unsigned char volatile *R0,unsigned char volatile *R1,unsigned char volatile *UMCTL){
  if(SYSCLK==8000000){
    switch (Baudrate){
      case 1200:
        *R1=0x1A;
        *R0=0x0A;
        *UMCTL=0x00;
        break;
      case 9600:
        *R1=0x03;
        *R0=0x41;
        *UMCTL=0x00;  
        break;
      case 19200:
        *R1=0x01;
        *R0=0xA0;
        *UMCTL=0xC0;
        break;
      case 57600:
        *R1=0x00;
        *R0=0x8A;
        *UMCTL=0xDE;
        break;
      case 115200:
        *R1=0x00;
        *R0=0x45;
        *UMCTL=0x4A;
        break;
    }
  
  }
}

void SetControlReg(unsigned char volatile *UCCTL1,unsigned char volatile *UCBR0,
                   unsigned char volatile *UCBR1,unsigned char volatile *UCMCTL,
                   unsigned char volatile *UCIE,unsigned long Baudrate)           {
  *UCCTL1 |= UCSSEL_2;                     // CLK = ACLK
  SetBaudrate(MSP430Clock,Baudrate,UCBR0,UCBR1,UCMCTL);
  *UCCTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  *UCIE |= UCRXIE;                        // Enable USCI_A1 RX interrupt
}


void OpenUart(unsigned char Port,unsigned long Baudrate,void (*CallBackFunction)(unsigned char)){
  switch (Port){
    case 0:
      P3SEL |= BIT4 | BIT5;
      SetControlReg(&UCA0CTL1,&UCA0BR0,&UCA0BR1,&UCA0MCTL,&UCA0IE,Baudrate); 
      COM0CallBack=CallBackFunction;
      break;
    case 1:
      P5SEL |= BIT6 | BIT7;
      SetControlReg(&UCA1CTL1,&UCA1BR0,&UCA1BR1,&UCA1MCTL,&UCA1IE,Baudrate); 
      COM1CallBack=CallBackFunction;
      break;
    case 2:
      P9SEL |= BIT4 | BIT5;
      SetControlReg(&UCA2CTL1,&UCA2BR0,&UCA2BR1,&UCA2MCTL,&UCA2IE,Baudrate);
      COM2CallBack=CallBackFunction;
      break;
    case 3:
      P10SEL = BIT4 | BIT5; 
      SetControlReg(&UCA3CTL1,&UCA3BR0,&UCA3BR1,&UCA3MCTL,&UCA3IE,Baudrate); 
      COM3CallBack=CallBackFunction;
      break;
  }
}

void SendTextToUart(unsigned char Port,const unsigned char *SBUF,unsigned int length){
  switch (Port){
    case 0:
      for (int i=0;i<length;i++){
        while (!(UCA0IFG&UCTXIFG));                                 // USART0 TX buffer ready?
        UCA0TXBUF = SBUF[i];
      }
      break;
    case 1:
      for (int i=0;i<length;i++){
        while (!(UCA1IFG&UCTXIFG));                                 // USART0 TX buffer ready?
        UCA1TXBUF = SBUF[i];
      }
      break;
    case 2:
      for (int i=0;i<length;i++){
        while (!(UCA2IFG&UCTXIFG));                                 // USART0 TX buffer ready?
        UCA2TXBUF = SBUF[i];
      }
      break;
    case 3:
      for (int i=0;i<length;i++){
        while (!(UCA3IFG&UCTXIFG));                                 // USART0 TX buffer ready?
        UCA3TXBUF = SBUF[i];
      }
      break;
  }

}

void SendByteToUart(unsigned char Port,unsigned char SBUF){
  switch (Port){
    case 0:
      while (!(UCA0IFG&UCTXIFG));                // USCI_A1 TX buffer ready?
      UCA0TXBUF = SBUF;                          // TX -> RXed character
      break;
    case 1:
      while (!(UCA1IFG&UCTXIFG));                // USCI_A1 TX buffer ready?
      UCA1TXBUF = SBUF;                          // TX -> RXed character
      break;
    case 2:
      while (!(UCA2IFG&UCTXIFG));                // USCI_A1 TX buffer ready?
      UCA2TXBUF = SBUF;                          // TX -> RXed character
      break;
    case 3:
      while (!(UCA3IFG&UCTXIFG));                // USCI_A1 TX buffer ready?
      UCA3TXBUF = SBUF;                          // TX -> RXed character
      break;
  }
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI0_ISR(void)
{
  unsigned char RX=UCA0RXBUF;
  COM0CallBack(RX);
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI1_ISR(void)
{
  unsigned char RX=UCA1RXBUF;
  COM1CallBack(RX);
}

#pragma vector=USCI_A2_VECTOR
__interrupt void USCI2_ISR(void)
{
  unsigned char RX=UCA2RXBUF;
  COM2CallBack(RX);
}

#pragma vector=USCI_A3_VECTOR
__interrupt void USCI3_ISR(void)
{
  unsigned char RX=UCA3RXBUF;
  COM3CallBack(RX);
}
