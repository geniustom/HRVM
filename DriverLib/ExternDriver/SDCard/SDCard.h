#ifndef SDCARD_H
#define SDCARD_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "msp430.h"
#include "HAL_SDCard.h"
#include "Fatfs/ff.h"

// Function Test
extern void WriteTest(void);
extern void ContinueWriteTest(void);
extern FRESULT WriteFile(char* fileName, char* text, WORD size);

#endif /* SDCARD_H */



