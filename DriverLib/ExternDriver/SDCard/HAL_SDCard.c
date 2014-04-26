/***************************************************************************//**
 * @file       HAL_SDCard.c
 * @addtogroup HAL_SDCard
 * @{
 ******************************************************************************/
#include "msp430.h"
#include "HAL_SDCard.h"

// Pins from MSP430 connected to the SD Card
#define SD_CLK          BIT0
#define SD_SIMO         BIT4
#define SD_SOMI         BIT5
#define SD_CS           BIT3

// Ports
#define SD_SEL          P10SEL
#define SD_DIR          P10DIR
#define SD_OUT          P10OUT
#define SD_REN          P10REN
#define SD_CS_SEL       P10SEL
#define SD_CS_OUT       P10OUT
#define SD_CS_DIR       P10DIR

// UCXX SPI Controller
#define SDUCxCTL0       UCA3CTL0
#define SDUCxCTL1       UCA3CTL1
#define SDUCxBR0        UCA3BR0
#define SDUCxBR1        UCA3BR1
#define SDUCxSTAT       UCA3STAT
#define SDUCxIFG        UCA3IFG
#define SDUCxTXUBF      UCA3TXBUF
#define SDUCxRXUBF      UCA3RXBUF

/***************************************************************************//**
 * @brief   Initialize SD Card
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_init(void)
{
    // Port initialization for SD Card operation
    SD_SEL |= SD_CLK + SD_SOMI + SD_SIMO;
    SD_DIR |= SD_CLK + SD_SIMO;
    SD_REN |= SD_SOMI;                                   // Pull-Ups on SD Card SOMI
    SD_OUT |= SD_SOMI;                                   // Certain SD Card Brands need pull-ups

    SD_CS_SEL &= ~SD_CS;
    SD_CS_OUT |= SD_CS;
    SD_CS_DIR |= SD_CS;

    // Initialize USCI_B1 for SPI Master operation
    SDUCxCTL1 |= UCSWRST;                                   // Put state machine in reset
    SDUCxCTL0 = UCCKPL + UCMSB + UCMST + UCMODE_0 + UCSYNC; // 3-pin, 8-bit SPI master
    // Clock polarity select - The inactive state is high
    // MSB first
    SDUCxCTL1 = UCSWRST + UCSSEL_2;                         // Use SMCLK, keep RESET
    SDUCxBR0 = 63;                                          // Initial SPI clock must be <400kHz
    SDUCxBR1 = 0;                                           // f_UCxCLK = 25MHz/63 = 397kHz
    SDUCxCTL1 &= ~UCSWRST;                                  // Release USCI state machine
    SDUCxIFG &= ~UCRXIFG;
}

/***************************************************************************//**
 * @brief   Enable fast SD Card SPI transfers. This function is typically
 *          called after the initial SD Card setup is done to maximize
 *          transfer speed.
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_fastMode(void)
{
    SDUCxCTL1 |= UCSWRST;                                   // Put state machine in reset
    SDUCxBR0 = 1;                                           // f_UCxCLK = 25MHz/2 = 12.5MHz
    SDUCxBR1 = 0;
    SDUCxCTL1 &= ~UCSWRST;                                  // Release USCI state machine
}

/***************************************************************************//**
 * @brief   Read a frame of bytes via SPI
 * @param   pBuffer Place to store the received bytes
 * @param   size Indicator of how many bytes to receive
 * @return  None
 ******************************************************************************/

void SDCard_readFrame(uint8_t *pBuffer, uint16_t size)
{
    uint16_t gie = __get_SR_register() & GIE;              // Store current GIE state

    __disable_interrupt();                                 // Make this operation atomic

    SDUCxIFG &= ~UCRXIFG;                                   // Ensure RXIFG is clear

    // Clock the actual data transfer and receive the bytes
    while (size--){
        while (!(SDUCxIFG & UCTXIFG)) ;                     // Wait while not ready for TX
        SDUCxTXUBF = 0xff;                                  // Write dummy byte
        while (!(SDUCxIFG & UCRXIFG)) ;                     // Wait for RX buffer (full)
        *pBuffer++ = SDUCxRXUBF;
    }

    __bis_SR_register(gie);                                // Restore original GIE state
}

/***************************************************************************//**
 * @brief   Send a frame of bytes via SPI
 * @param   pBuffer Place that holds the bytes to send
 * @param   size Indicator of how many bytes to send
 * @return  None
 ******************************************************************************/

void SDCard_sendFrame(uint8_t *pBuffer, uint16_t size)
{
    uint16_t gie = __get_SR_register() & GIE;              // Store current GIE state

    __disable_interrupt();                                 // Make this operation atomic

    // Clock the actual data transfer and send the bytes. Note that we
    // intentionally not read out the receive buffer during frame transmission
    // in order to optimize transfer speed, however we need to take care of the
    // resulting overrun condition.
    while (size--){
        while (!(SDUCxIFG & UCTXIFG)) ;                     // Wait while not ready for TX
        SDUCxTXUBF = *pBuffer++;                            // Write byte
    }
    while (SDUCxSTAT & UCBUSY) ;                            // Wait for all TX/RX to finish

    SDUCxRXUBF;                                             // Dummy read to empty RX buffer
                                                           // and clear any overrun conditions

    __bis_SR_register(gie);                                // Restore original GIE state
}

/***************************************************************************//**
 * @brief   Set the SD Card's chip-select signal to high
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_setCSHigh(void)
{
    SD_CS_OUT |= SD_CS;
}

/***************************************************************************//**
 * @brief   Set the SD Card's chip-select signal to low
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_setCSLow(void)
{
    SD_CS_OUT &= ~SD_CS;
}

/***************************************************************************//**
 * @}
 ******************************************************************************/


