#ifndef HAL_SDCARD_H
#define HAL_SDCARD_H

#include "msp430.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern void SDCard_init(void);
extern void SDCard_fastMode(void);
extern void SDCard_readFrame(uint8_t *pBuffer, uint16_t size);
extern void SDCard_sendFrame(uint8_t *pBuffer, uint16_t size);
extern void SDCard_setCSHigh(void);
extern void SDCard_setCSLow(void);

// Function Test
extern void WriteTest(void);

#endif /* HAL_SDCARD_H */
