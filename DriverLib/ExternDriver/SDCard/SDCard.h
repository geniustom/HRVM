#ifndef SDCARD_H
#define SDCARD_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "msp430.h"
#include "HAL_SDCard.h"
#include "Fatfs/ff.h"

extern void SDCard(void);
extern void WriteTest(void);
extern void ContinueWriteTest(void);


#endif /* SDCARD_H */



