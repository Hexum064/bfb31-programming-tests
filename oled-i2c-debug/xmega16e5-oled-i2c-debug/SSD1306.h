/*
 * SSD1306.h
 *
 *  Created on: April 8, 2020
 *      Author: Michael Bolt
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>
#include "i2c.h"

/********************
 * SSD1306 Hardware *
 ********************/
// Hardware description
#define SSD1306_I2C_ADDRESS 0x3C    // default I2C address
#define SSD1306_ROWS        32      // number of rows on display
#define SSD1306_COLUMNS     128     // number of columns on display
#define SSD1306_PAGE_START  0
#define SSD1306_PAGE_STOP   ((SSD1306_ROWS / 8) - 1)
#define SSD1306_COL_START   0
#define SSD1306_COL_STOP    (SSD1306_COLUMNS - 1)
// SSD1306 Commands - see Datasheet
#define SSD1306_CMD_START   0x00    // indicates following bytes are commands
#define SSD1306_DATA_START  0x40    // indicates following bytes are data
// Fundamental Command Table (p. 28)
#define SSD1306_SETCONTRAST         0x81    // double-byte command to set contrast (1-256)
#define SSD1306_ENTIREDISPLAY_ON    0xA5    // set entire display on
#define SSD1306_ENTIREDISPLAY_OFF   0xA4    // use RAM contents for display
#define SSD1306_SETINVERT_ON        0xA7    // invert RAM contents to display
#define SSD1306_SETINVERT_OFF       0xA6    // normal display
#define SSD1306_SETDISPLAY_OFF      0xAE    // display OFF (sleep mode)
#define SSD1306_SETDISPLAY_ON       0xAF    // display ON (normal mode)
// Scrolling Command Table (pp. 28-30)
#define SSD1306_SCROLL_SETUP_H_RIGHT    0x26    // configure right horizontal scroll
#define SSD1306_SCROLL_SETUP_H_LEFT     0x27    // configure left horizontal scroll
#define SSD1306_SCROLL_SETUP_HV_RIGHT   0x29    // configure right & vertical scroll
#define SSD1306_SCROLL_SETUP_HV_LEFT    0x2A    // configure left & vertical scroll
#define SSD1306_SCROLL_SETUP_V          0xA3    // configure vertical scroll area
#define SSD1306_SCROLL_DEACTIVATE       0x2E    // stop scrolling
#define SSD1306_SCROLL_ACTIVATE         0x2F    // start scrolling
// Addressing Setting Command Table (pp. 30-31)
#define SSD1306_PAGE_COLSTART_LOW   0x00    // set lower 4 bits of column start address by ORing 4 LSBs
#define SSD1306_PAGE_COLSTART_HIGH  0x10    // set upper 4 bits of column start address by ORing 4 LSBs
#define SSD1306_PAGE_PAGESTART      0xB0    // set page start address by ORing 4 LSBs
#define SSD1306_SETADDRESSMODE      0x20    // set addressing mode (horizontal, vertical, or page)
#define SSD1306_SETCOLRANGE         0x21    // send 2 more bytes to set start and end columns for hor/vert modes
#define SSD1306_SETPAGERANGE        0x22    // send 2 more bytes to set start and end pages
// Hardware Configuration Commands (p. 31)
#define SSD1306_SETSTARTLINE        0x40    // set RAM display start line by ORing 6 LSBs
#define SSD1306_COLSCAN_ASCENDING   0xA0    // set column address 0 to display column 0
#define SSD1306_COLSCAN_DESCENDING  0xA1    // set column address 127 to display column 127
#define SSD1306_SETMULTIPLEX        0xA8    // set size of multiplexer based on display height (31 for 32 rows)
#define SSD1306_COMSCAN_ASCENDING   0xC0    // set COM 0 to display row 0
#define SSD1306_COMSCAN_DESCENDING  0xC8    // set COM N-1 to display row 0
#define SSD1306_VERTICALOFFSET      0xD3    // set display vertical shift
#define SSD1306_SETCOMPINS          0xDA    // set COM pin hardware configuration
// Timing and Driving Scheme Settings Commands (p. 32)
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5    // set display clock divide ratio and frequency
#define SSD1306_SETPRECHARGE        0xD9    // set pre-charge period
#define SSD1306_SETVCOMLEVEL        0xDB    // set V_COMH voltage level
#define SSD1306_NOP                 0xE3    // no operation
// Charge Pump Commands (p. 62)
#define SSD1306_SETCHARGEPUMP       0x8D    // enable / disable charge pump

/***************************************************************************************************
 Macros to find the mod of a number
 ****************************************************************************************************/
#define util_GetMod8(dividend, divisor) (uint8_t)(dividend - (divisor * (uint8_t)(dividend / divisor)))
#define util_GetMod16(dividend, divisor) (uint16_t)(dividend - (divisor * (uint16_t)(dividend / divisor)))
#define util_GetMod32(dividend, divisor) (uint32_t)(dividend - (divisor * (uint32_t)(dividend / divisor)))
/***************************************************************************************************/

/***************************************************************************************************
 Macros for Dec2Ascii,Hec2Ascii and Acsii2Hex conversion
 *****************************************************************************************************/
#define util_Dec2Ascii(Dec) ((Dec) + 0x30)
#define util_Ascii2Dec(Asc) ((Asc)-0x30)
#define util_Hex2Ascii(Hex) (((Hex) > 0x09) ? ((Hex) + 0x37) : ((Hex) + 0x30))
#define util_Ascii2Hex(Asc) (((Asc) > 0x39) ? ((Asc)-0x37) : ((Asc)-0x30))
#define util_GetBitStatus(x, bit) (((x) & (util_GetBitMask(bit))) != 0u)
#define util_GetBitMask(bit) (1 << (bit))
/***************************************************************************************************/

/******************************************************************************
 standard defs
 ******************************************************************************/

typedef signed char sint8_t;
typedef unsigned char uint8_t;
typedef signed int sint16_t;
typedef signed long int sint32_t;

#define C_BINARY_U8 2u
#define C_DECIMAL_U8 10u
#define C_HEX_U8 16u

#define C_SINT8_MAX 0x7F
#define C_SINT8_MIN -128

#define C_UINT8_MAX 0xFFu
#define C_UINT8_MIN 0x00u

#define C_SINT16_MAX 32767
#define C_SINT16_MIN -32768

#define C_UINT16_MAX 0xFFFFu
#define C_UINT16_MIN 0x00u

#define C_SINT32_MAX 2147483647
#define C_SINT32_MIN -2147483648

#define C_UINT32_MAX 0xFFFFFFFFu
#define C_UINT32_MIN 0x00u

typedef enum
{
    E_BINARY = 2,
    E_DECIMAL = 10,
    E_HEX = 16
} NumericSystem_et;


/***************************************************************************************************
 Commonly used LCD macros/Constants
 ***************************************************************************************************/
#define C_DefaultDigits 10

#define C_SSD1306FirstLine_U8 0x00u
#define C_SSD1306LastLine_U8 0x07u

#define C_FirstLineAddress_U8 0xB8
#define C_LastLineAddress_U8 0xBF

#define C_DisplayDefaultDigits_U8 0xffu                              // Will display the exact digits in the number
#define C_MaxDigitsToDisplay_U8 10u                                  // Max decimal/hexadecimal digits to be displayed
#define C_NumOfBinDigitsToDisplay_U8 16u                             // Max bits of a binary number to be displayed
#define C_MaxDigitsToDisplayUsingPrintf_U8 C_DisplayDefaultDigits_U8 /* Max dec/hexadecimal digits to be displayed using printf */

#define C_MaxBarGraphs_U8 5

#define FONT_SIZE 5

// function definitions
uint16_t ssd1306_init(void);
uint16_t ssd1306_drawPixel(uint16_t x, uint16_t y, uint8_t value);



/*****************************
 * Dirty Rectangle Animation *
 *****************************/
// length of buffer will be 2 ^ DIRTY_RECT_BUFFER_POWER
#define DIRTY_RECT_BUFFER_POWER 5
// Circular Buffer struct definition
typedef struct DirtyRectangleBuffer_S {
    uint16_t    readIndex;
    uint16_t    writeIndex;
    uint16_t    length;
    uint8_t     coordinates[2][1 << DIRTY_RECT_BUFFER_POWER];
} DirtyRectangleBuffer;
// function definitions
uint16_t dirtyRect_write(uint8_t x, uint8_t y);
uint16_t dirtyRect_read(uint8_t *x, uint8_t *y);



/*************************
 * Display Functionality *
 *************************/
// max length of list will be 4 * Dirty Rect Animation buffer list
#define DISPLAY_REGION_LIST_POWER (DIRTY_RECT_BUFFER_POWER + 2)
// Display Region List struct definition
typedef struct DisplayRegionList_S {
    uint16_t    length;
    uint8_t     coordinates[2][1 << DISPLAY_REGION_LIST_POWER];
} DisplayRegionList;
// function definitions
void display_frameStart(void);
uint16_t display_drawSprite(uint16_t x, uint16_t y, const uint8_t *const sprite);
uint16_t display_drawFrame(void);
uint16_t display_clearScreen(void);
void ssd1306_displayChar(uint8_t ch);
void ssd1306_displayString(uint8_t *ptr);
void  ssd1306_goToLine(uint8_t lineNumber);
void  ssd1306_goToNextLine();
void ssd1306_setCursor(uint8_t lineNumber,uint8_t cursorPosition);
void ssd1306__printf(const char *argList, ...);
void ssd1306_displayFloatNumber(double v_floatNum_f32);
void ssd1306_displayNumber(uint8_t v_numericSystem_u8, uint32_t v_number_u32, uint8_t v_numOfDigitsToDisplay_u8);
void ssd1306_clear();

#endif /* SSD1306_H_ */