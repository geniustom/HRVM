/*******************************************************************************
 *
 *  SDCard.c - Accesses the SD Card
 *
 *  Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/***************************************************************************//**
 * @file       SDCard.c
 * @addtogroup SDCard
 * @{
 ******************************************************************************/
#include "SDCard.h"


#define NAMES_SIZE  200
#define LINE_SIZE   17
#define PATH_SIZE   50
#define PATH_LENGTHS_SIZE 10
#define NAME_LENGTHS_SIZE 50

char names[NAMES_SIZE];
char output[LINE_SIZE];
char path[PATH_SIZE];
uint8_t pathLengths[PATH_LENGTHS_SIZE];
uint8_t nameLengths[NAME_LENGTHS_SIZE];
uint8_t numNames = 0;
uint8_t namesIndex = 0;
uint8_t listIndex = 0;
uint8_t selectedIndex = 0;
uint8_t selectedLength = 0;

char buff[255]; //255



/***************************************************************************//**
 * @brief   Entry point into the User Experience's SD Card demo
 * @param   None
 * @return  None
 ******************************************************************************/



//==========================================WriteTest=======================
int FileIndex=0;
char F[6]={'0','.','t','x','t',0x00};
void WriteTest(void)
{
  char SDData[6]={0x30,0x30,0x30,0x30,0x0D,0x0A};
  F[0]=0x30+(FileIndex%10);
  for(int i=0;i<4;i++){
    SDData[i]=0x30+FileIndex;
  }
  WriteFile(F, SDData, 6);
  FileIndex++;
}

//==========================================Die=======================
void die(FRESULT rc){
  _NOP();
}
//====================================ContinueWriteTest=====================
void ContinueWriteTest(void){
  int a=0x00;
  while (1){        //while (!(buttonsPressed & BUTTON_S1)){
    WriteFile("A.txt", "a", 1);
    a++;
    a%=2;
    if(a%2==0){
      //Board_ledOn(LED1);
    }else{
      //Board_ledOff(LED1);
    }
    //buttonsPressed = 0;
  }
}


/***************************************************************************//**
 * @brief   Get the current file and store it
 * @param   None
 * @return  None
 ******************************************************************************/

FRESULT GetFile(char* fileName)
{
    FRESULT rc;                                            /* Result code */
    FATFS fatfs;                                           /* File system object */
    FIL fil;                                               /* File object */
    UINT br;

    //Board_ledOn(LED1);                                     //turn on red LED to show we are
                                                           // accessing the SD card

    f_mount(0, &fatfs);                                    /* Register volume work area (never
                                                            *fails) */

    rc = f_open(&fil, fileName, FA_READ);

    for (;;)
    {
        rc = f_read(&fil, buff, sizeof(buff), &br);        /* Read a chunk of file */
        if (rc || !br) break;                              /* Error or end of file */
    }
    if (rc)                                                //if error
    {
        f_close(&fil);
        //Board_ledOff(LED1);                                //turn off red LED to show we are done
                                                           // accessing the SD card
        return rc;
    }

    rc = f_close(&fil);
    //Board_ledOff(LED1);                                    //turn off red LED to show we are done
                                                           // accessing the SD card
    return rc;
}

/***************************************************************************//**
 * @brief   This function will create a new file, writes, and close the file.
 *          This function will overwrite your file.
 * @param   fileName Specify the filename of the file
 * @param   text     Data to write to file
 * @param   size     Data size to be written
 * @return  None
 ******************************************************************************/

FRESULT WriteFile(char* fileName, char* text, WORD size)
{
    // Result code
    FRESULT rc;
    // File system object
    FATFS fatfs;
    // File object
    FIL fil;
    UINT bw;

    // Register volume work area (never fails)
    f_mount(0, &fatfs);

    // Open file
    //rc = f_open(&fil, fileName, FA_WRITE | FA_CREATE_ALWAYS);
    //rc = f_open(&fil, fileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE );
    rc = f_open(&fil, fileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE );
    if (rc)     die(rc);

    // Seek file
    rc = f_lseek(&fil, fil.fsize);
    if (rc)     die(rc);    
    
    // Write to file
    rc = f_write(&fil, text, size, &bw);
    //rc = f_puts(text,&fil);
    if (rc)     die(rc);

    // Close the file
    rc = f_close(&fil);
    if (rc)     die(rc);

    return rc;
}


/***************************************************************************//**
 * @brief   Clears the data structures storing the data read from the card
 * @param   None
 * @return  None
 ******************************************************************************/

void ClearTempData(void)
{
    uint8_t i = 0;

    for (i = 0; i < 255; i++)
    {
        buff[i] = 0;
    }
    for (i = 0; i < NAMES_SIZE; i++)
    {
        names[i] = 0;
    }
    for (i = 0; i < 17; i++)
    {
        output[i] = 0;
    }
    for (i = 0; i < NAME_LENGTHS_SIZE; i++)
    {
        nameLengths[i] = 0;
    }
    numNames = 0;
    namesIndex = 0;
    listIndex = 0;
}


/***************************************************************************//**
 * @}
 ******************************************************************************/

