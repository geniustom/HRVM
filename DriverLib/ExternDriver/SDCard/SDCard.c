
/***************************************************************************//**
 * @file       SDCard.c
 * @addtogroup SDCard
 * @{
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "msp430.h"
#include "HAL_SDCard.h"
#include "SDCard.h"
#include "Fatfs/ff.h"



// This image has been created by ImageDog using 'Safety_Yellow_DangerGeneral.png'
static const uint8_t warningSign[] =
{
    0x30,    // Image width in pixels
    0x06,    // Image height in rows (1 row = 8 pixels)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x03, 0x04, 0x13, 0x6f, 0x1c, 0xb8, 0x5e, 0x27, 0x1b, 0x06, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06,
    0x09, 0x27, 0xde, 0x38, 0xf0, 0xc3, 0x07, 0x0f, 0x0f, 0x0f, 0x87, 0xc0, 0xf0, 0x3c, 0x4f, 0x13,
    0x0c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0c, 0x13, 0x4f, 0xbc, 0x70,
    0xe0, 0x80, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xc0,
    0xf0, 0x78, 0x9e, 0x27, 0x19, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x19, 0x27, 0x9e, 0x78, 0xf0, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xfc, 0xfc, 0xfc, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0xe0, 0x78, 0x1e, 0x47, 0x13, 0x0c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x0d, 0x13, 0x4f, 0x3c, 0xf0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xf0, 0x3c, 0x8f, 0x23, 0x08, 0x06, 0x01,
    0xe0, 0x50, 0xf0, 0xe0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0xe0, 0xe0, 0x10, 0xe0
};

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


void ActiveDirectory(void);
void ActiveFile(void);
void SDDisplay(uint8_t position);
void ClearTempData(void);

void die(FRESULT rc);

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
  char Data[6]={0x30,0x30,0x30,0x30,0x0D,0x0A};
  F[0]=0x30+(FileIndex%10);
  for(int i=0;i<4;i++){
    Data[i]=0x30+FileIndex;
  }
  WriteFile(F, Data, 6);
  FileIndex++;
}
//====================================ContinueWriteTest=====================
void ContinueWriteTest(void){
  int a=0;
  char c[750]={0};
  for(int i=0;i<750;i++){
    c[i]='x';
  }
  while (!(buttonsPressed & BUTTON_S2)){
    WriteFile("A.txt", c, 750);
    a++;
    if(a%2==0){
      Board_ledOn(LED1);
    }else{
      Board_ledOff(LED1);
    }
    buttonsPressed = 0;
  }
}
//==========================================================================
void SDCard(void)
{
    FRESULT rc;
    uint8_t a = 0, b = 0;
    uint8_t level = 0;

    for (a = 0; a < PATH_SIZE; a++)
        path[a] = 0;                                            //clear string
    for (a = 0; a < PATH_LENGTHS_SIZE; a++)
        pathLengths[a] = 0;                                     //clear string
    rc = GetDirectory("");                                      //Get the root directory listing
    if (rc) die(rc);                                            //If no SD card, show warning
    else
    {
        // Re-initialize SPI port for LCD usage before returning to the main menu
        Dogs102x6_init();
        Dogs102x6_clearScreen();
        ActiveDirectory();

        while (!((level == 0) && (buttonsPressed & BUTTON_S2))) //continue while not button S2 in
                                                                // root directory
        {
            if (buttonsPressed & BUTTON_S1)                     //go to next level
            {
                buttonsPressed = 0;

                level++;
                pathLengths[level] = selectedLength - 4 + 1;    //remove first 4 chars, but add 1
                                                                // for "/" character
                strcat(path, "/");
                strncat(path, names + selectedIndex + 4, selectedLength - 4);

                if (names[selectedIndex] == '<')                //check if it's a directory
                {
                    GetDirectory(path);
                    Dogs102x6_init();
                    Dogs102x6_clearScreen();
                    ActiveDirectory();
                }
                else //it's a file
                {
                    rc = GetFile(path);
                    if (rc)                                     //if error in getting file
                    {
                        //remove last bit of path
                        b = 0;
                        for (a = 0; a < level; a++)
                            b += pathLengths[a];
                        for (a = b; a < PATH_SIZE; a++)
                            path[a] = 0;
                        pathLengths[level] = 0;
                        level--;
                    }
                    Dogs102x6_init();
                    Dogs102x6_clearScreen();
                    ActiveFile();
                }
            }
            else //back out one level
            {
                buttonsPressed = 0;

                //remove last bit of path
                b = 0;
                for (a = 0; a < level; a++)
                    b += pathLengths[a];
                for (a = b; a < PATH_SIZE; a++)
                    path[a] = 0;
                pathLengths[level] = 0;
                level--;

                if (level != 0) //not root directory
                {
                    GetDirectory(path);
                    Dogs102x6_init();
                    Dogs102x6_clearScreen();
                    ActiveDirectory();
                }
                else //root directory
                {
                    GetDirectory("");
                    Dogs102x6_init();
                    Dogs102x6_clearScreen();
                    ActiveDirectory();
                }
            }
        }
    }

    ClearTempData();
    buttonsPressed = 0;
    Dogs102x6_clearScreen(); //clear the screen
}


/***************************************************************************//**
 * @brief   Display the current directory listing and allow user to select files
 *          and directories to view
 * @param   None
 * @return  None
 ******************************************************************************/

void ActiveDirectory(void)
{
    uint8_t position = 0, lastPosition = 9;

    listIndex = 0;
    buttonsPressed = 0;
    while (!buttonsPressed)
    {
        position = Wheel_getPosition();
        if ((position == 7) && (listIndex < numNames - 1)) //if at bottom of display, scroll down,
                                                           // if possible
        {
            namesIndex += nameLengths[listIndex];
            listIndex++;
            Dogs102x6_clearScreen();
            SDDisplay(position);
            __delay_cycles(1500000);                       //delay for user to see scrolling
        }
        else if ((position <= 1) && (listIndex != 0))      //if at top of display scroll up, if
                                                           // possible
        {
            listIndex--;
            namesIndex -= nameLengths[listIndex];
            Dogs102x6_clearScreen();
            SDDisplay(position);
            __delay_cycles(1500000);                       //delay for user to see scrolling
        }
        else if (position != lastPosition)                 //update position on display
        {
            SDDisplay(position);
            lastPosition = position;
        }
    }
}

/***************************************************************************//**
 * @brief   Display the current file
 * @param   None
 * @return  None
 ******************************************************************************/

void ActiveFile(void)
{
    uint8_t index = 0, position = 0;
    char screen[136];

    //display first screen
    strncpy(screen, buff + index, 136);                        //we can fit 136 characters on our
                                                               // screen
    Dogs102x6_clearScreen();
    Dogs102x6_stringDraw(0, 0, screen, DOGS102x6_DRAW_NORMAL);

    while (!buttonsPressed)
    {
        position = Wheel_getPosition();
        if ((position == 7) && (buff[index + LINE_SIZE] != 0)) //scroll down, if possible
        {
            index += 17;                                       //move screen down one line
            strncpy(screen, buff + index, 136);                //we can fit 136 characters on our
                                                               // screen
            Dogs102x6_clearScreen();
            Dogs102x6_stringDraw(0, 0, screen, DOGS102x6_DRAW_NORMAL);
            __delay_cycles(1500000);                           //delay for user to see scrolling
        }
        else if ((position <= 1) && (index != 0))              //scroll up, if possible
        {
            index -= 17;                                       //move screen up one line
            strncpy(screen, buff + index, 136);                //we can fit 136 characters on our
                                                               // screen
            Dogs102x6_clearScreen();
            Dogs102x6_stringDraw(0, 0, screen, DOGS102x6_DRAW_NORMAL);
            __delay_cycles(1500000);                           //delay for user to see scrolling
        }
    }
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
 * @brief   Displays one screen of the listings
 * @param   position  Position of the wheel
 * @return  None
 ******************************************************************************/

void SDDisplay(uint8_t position)
{
    uint8_t k = 8, a = 0, i, j;

    i = listIndex;
    j = namesIndex;

    if ((listIndex + position) >= numNames)
        position = numNames - listIndex - 1;

    //display one screen of information
    for (k = 0; k < 8; k++)
    {
        if (i < numNames)
        {
            for (a = 0; a < 17; a++)
                output[a] = 0;     //clear string

            strncpy(output, names + j, nameLengths[i]);
            if (k == position)     //highlight current position
            {
                Dogs102x6_stringDraw(k, 0, output, DOGS102x6_DRAW_INVERT);
                selectedIndex = j; //location of currently selected file/directory
                selectedLength = nameLengths[i];
            }
            else
            {
                Dogs102x6_stringDraw(k, 0, output, DOGS102x6_DRAW_NORMAL);
            }
            j += nameLengths[i];
            i++;
        }
        else
        {
            Dogs102x6_clearRow(k); //no more files, so print blank line
        }
    }
}

/***************************************************************************//**
 * @brief   Stop with dying message, part of the FatFs sample project by ChaN
 * @param   rc  FatFs return value
 * @return  None
 ******************************************************************************/

void die(FRESULT rc)
{
    buttonsPressed = 0;
    Dogs102x6_init();
    Dogs102x6_clearScreen();
    Dogs102x6_imageDraw(warningSign, 2, 29);
    Dogs102x6_stringDraw(0, 27, "WARNING!", DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(1, 0, "NO SD CARD FOUND!", DOGS102x6_DRAW_NORMAL);
    while (!buttonsPressed)
    {
        // Wait in low power mode 3 until a button is pressed
        __bis_SR_register(LPM3_bits + GIE);
    }
}

/***************************************************************************//**
 * @}
 ******************************************************************************/
