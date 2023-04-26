/*
 * SSD1306.c
 *
 *  Created on: April 8, 2020
 *      Author: Michael Bolt
 */

#include "SSD1306.h"
#include <stdio.h>
#include <stdarg.h>

/**************************************************************
 *  SSD1306 Functions  ****************************************
 **************************************************************/
static uint8_t  ssd1306_vram[SSD1306_ROWS / 8][SSD1306_COLUMNS] = {0};
uint8_t ssd1306_lineNum, ssd1306_cursorPos;

const unsigned char OledFontTable[][FONT_SIZE]={
	
	0x00, 0x00, 0x00, 0x00, 0x00,   // space
	0x00, 0x00, 0x2f, 0x00, 0x00,   // !
	0x00, 0x07, 0x00, 0x07, 0x00,   // "
	0x14, 0x7f, 0x14, 0x7f, 0x14,   // #
	0x24, 0x2a, 0x7f, 0x2a, 0x12,   // $
	0x23, 0x13, 0x08, 0x64, 0x62,   // %
	0x36, 0x49, 0x55, 0x22, 0x50,   // &
	0x00, 0x05, 0x03, 0x00, 0x00,   // '
	0x00, 0x1c, 0x22, 0x41, 0x00,   // (
	0x00, 0x41, 0x22, 0x1c, 0x00,   // )
	0x14, 0x08, 0x3E, 0x08, 0x14,   // *
	0x08, 0x08, 0x3E, 0x08, 0x08,   // +
	0x00, 0x00, 0xA0, 0x60, 0x00,   // ,
	0x08, 0x08, 0x08, 0x08, 0x08,   // -
	0x00, 0x60, 0x60, 0x00, 0x00,   // .
	0x20, 0x10, 0x08, 0x04, 0x02,   // /
	
	0x3E, 0x51, 0x49, 0x45, 0x3E,   // 0
	0x00, 0x42, 0x7F, 0x40, 0x00,   // 1
	0x42, 0x61, 0x51, 0x49, 0x46,   // 2
	0x21, 0x41, 0x45, 0x4B, 0x31,   // 3
	0x18, 0x14, 0x12, 0x7F, 0x10,   // 4
	0x27, 0x45, 0x45, 0x45, 0x39,   // 5
	0x3C, 0x4A, 0x49, 0x49, 0x30,   // 6
	0x01, 0x71, 0x09, 0x05, 0x03,   // 7
	0x36, 0x49, 0x49, 0x49, 0x36,   // 8
	0x06, 0x49, 0x49, 0x29, 0x1E,   // 9
	
	0x00, 0x36, 0x36, 0x00, 0x00,   // :
	0x00, 0x56, 0x36, 0x00, 0x00,   // ;
	0x08, 0x14, 0x22, 0x41, 0x00,   // <
	0x14, 0x14, 0x14, 0x14, 0x14,   // =
	0x00, 0x41, 0x22, 0x14, 0x08,   // >
	0x02, 0x01, 0x51, 0x09, 0x06,   // ?
	0x32, 0x49, 0x59, 0x51, 0x3E,   // @
	
	0x7C, 0x12, 0x11, 0x12, 0x7C,   // A
	0x7F, 0x49, 0x49, 0x49, 0x36,   // B
	0x3E, 0x41, 0x41, 0x41, 0x22,   // C
	0x7F, 0x41, 0x41, 0x22, 0x1C,   // D
	0x7F, 0x49, 0x49, 0x49, 0x41,   // E
	0x7F, 0x09, 0x09, 0x09, 0x01,   // F
	0x3E, 0x41, 0x49, 0x49, 0x7A,   // G
	0x7F, 0x08, 0x08, 0x08, 0x7F,   // H
	0x00, 0x41, 0x7F, 0x41, 0x00,   // I
	0x20, 0x40, 0x41, 0x3F, 0x01,   // J
	0x7F, 0x08, 0x14, 0x22, 0x41,   // K
	0x7F, 0x40, 0x40, 0x40, 0x40,   // L
	0x7F, 0x02, 0x0C, 0x02, 0x7F,   // M
	0x7F, 0x04, 0x08, 0x10, 0x7F,   // N
	0x3E, 0x41, 0x41, 0x41, 0x3E,   // O
	0x7F, 0x09, 0x09, 0x09, 0x06,   // P
	0x3E, 0x41, 0x51, 0x21, 0x5E,   // Q
	0x7F, 0x09, 0x19, 0x29, 0x46,   // R
	0x46, 0x49, 0x49, 0x49, 0x31,   // S
	0x01, 0x01, 0x7F, 0x01, 0x01,   // T
	0x3F, 0x40, 0x40, 0x40, 0x3F,   // U
	0x1F, 0x20, 0x40, 0x20, 0x1F,   // V
	0x3F, 0x40, 0x38, 0x40, 0x3F,   // W
	0x63, 0x14, 0x08, 0x14, 0x63,   // X
	0x07, 0x08, 0x70, 0x08, 0x07,   // Y
	0x61, 0x51, 0x49, 0x45, 0x43,   // Z
	
	0x00, 0x7F, 0x41, 0x41, 0x00,   // [
	0x55, 0xAA, 0x55, 0xAA, 0x55,   // Backslash (Checker pattern)
	0x00, 0x41, 0x41, 0x7F, 0x00,   // ]
	0x04, 0x02, 0x01, 0x02, 0x04,   // ^
	0x40, 0x40, 0x40, 0x40, 0x40,   // _
	0x00, 0x03, 0x05, 0x00, 0x00,   // `
	
	0x20, 0x54, 0x54, 0x54, 0x78,   // a
	0x7F, 0x48, 0x44, 0x44, 0x38,   // b
	0x38, 0x44, 0x44, 0x44, 0x20,   // c
	0x38, 0x44, 0x44, 0x48, 0x7F,   // d
	0x38, 0x54, 0x54, 0x54, 0x18,   // e
	0x08, 0x7E, 0x09, 0x01, 0x02,   // f
	0x18, 0xA4, 0xA4, 0xA4, 0x7C,   // g
	0x7F, 0x08, 0x04, 0x04, 0x78,   // h
	0x00, 0x44, 0x7D, 0x40, 0x00,   // i
	0x40, 0x80, 0x84, 0x7D, 0x00,   // j
	0x7F, 0x10, 0x28, 0x44, 0x00,   // k
	0x00, 0x41, 0x7F, 0x40, 0x00,   // l
	0x7C, 0x04, 0x18, 0x04, 0x78,   // m
	0x7C, 0x08, 0x04, 0x04, 0x78,   // n
	0x38, 0x44, 0x44, 0x44, 0x38,   // o
	0xFC, 0x24, 0x24, 0x24, 0x18,   // p
	0x18, 0x24, 0x24, 0x18, 0xFC,   // q
	0x7C, 0x08, 0x04, 0x04, 0x08,   // r
	0x48, 0x54, 0x54, 0x54, 0x20,   // s
	0x04, 0x3F, 0x44, 0x40, 0x20,   // t
	0x3C, 0x40, 0x40, 0x20, 0x7C,   // u
	0x1C, 0x20, 0x40, 0x20, 0x1C,   // v
	0x3C, 0x40, 0x30, 0x40, 0x3C,   // w
	0x44, 0x28, 0x10, 0x28, 0x44,   // x
	0x1C, 0xA0, 0xA0, 0xA0, 0x7C,   // y
	0x44, 0x64, 0x54, 0x4C, 0x44,   // z
	
	0x00, 0x10, 0x7C, 0x82, 0x00,   // {
	0x00, 0x00, 0xFF, 0x00, 0x00,   // |
	0x00, 0x82, 0x7C, 0x10, 0x00,   // }
	0x00, 0x06, 0x09, 0x09, 0x06    // ~ (Degrees)
};

/*
 * Initialize the SSD1306 display module
 *
 * \return 0 if I2C communication was successful, otherwise the number
 *         of bytes left in the I2C transmission buffer when the
 *         exchange failed will be returned
 */
uint16_t ssd1306_init(void){
    const uint8_t instructions[] = {
        SSD1306_CMD_START,              // start commands
        SSD1306_SETDISPLAY_OFF,         // turn off display
        SSD1306_SETDISPLAYCLOCKDIV,     // set clock:
        0x80,                           //   Fosc = 8, divide ratio = 0+1
        SSD1306_SETMULTIPLEX,           // display multiplexer:
        (SSD1306_ROWS - 1),             //   number of display rows
        SSD1306_VERTICALOFFSET,         // display vertical offset:
        0,                              //   no offset
        SSD1306_SETSTARTLINE | 0x00,    // RAM start line 0
        SSD1306_SETCHARGEPUMP,          // charge pump:
        0x14,                           //   charge pump ON (0x10 for OFF)
        SSD1306_SETADDRESSMODE,         // addressing mode:
        0x00,                           //   horizontal mode
        SSD1306_COLSCAN_ASCENDING,     // flip columns
        SSD1306_COMSCAN_ASCENDING,      // don't flip rows (pages)
        SSD1306_SETCOMPINS,             // set COM pins
        0x02,                           //   sequential pin mode
        SSD1306_SETCONTRAST,            // set contrast
        0x00,                           //   minimal contrast
        SSD1306_SETPRECHARGE,           // set precharge period
        0xF1,                           //   phase1 = 15, phase2 = 1
        SSD1306_SETVCOMLEVEL,           // set VCOMH deselect level
        0x40,                           //   ????? (0,2,3)
        SSD1306_ENTIREDISPLAY_OFF,      // use RAM contents for display
        SSD1306_SETINVERT_OFF,          // no inversion
        SSD1306_SCROLL_DEACTIVATE,      // no scrolling
        SSD1306_SETDISPLAY_ON,          // turn on display (normal mode)
    };
    // send list of commands
     return twi_write(SSD1306_I2C_ADDRESS, instructions, sizeof instructions);
}


/*
 * ! Draw a single pixel to the display
 * !
 * ! \param x: x coordinate of pixel to write to [0:SSD1306_COLUMNS-1]
 * ! \param y: y coordinate of pixel to write to [0:SSD1306_ROWS-1]
 * ! \param value: value to write to the pixel [0:1]
 * !
 * ! \return 0 if successful, error code if failed:
 * !         1: x value out of range
 * !         2: y value out of range
 * !         3: I2C error during configuration
 * !         4: I2C error during data transmission
 */
uint16_t ssd1306_drawPixel(uint16_t x, uint16_t y, uint8_t value){
    // ensure pixel location is valid
    if (x >= SSD1306_COLUMNS)   return 1;
    if (y >= SSD1306_ROWS)      return 2;

    // send configuration message
    const uint8_t page = y >> 3;
    const uint8_t configMsg[] = {
        SSD1306_CMD_START,          // start commands
        SSD1306_SETPAGERANGE,       // set page range:
        page,                       //   y / 8
        page,                       //   y / 8
        SSD1306_SETCOLRANGE,        // set column range:
        x,                          //   x
        x                           //   x
    };
    if (twi_write(SSD1306_I2C_ADDRESS, configMsg, sizeof configMsg))   return 3;

    // draw pixel to VRAM
    if(value)   ssd1306_vram[page][x] |=   0x01 << (y & 0x07);
    else        ssd1306_vram[page][x] &= ~(0x01 << (y & 0x07));

    // draw updated VRAM page to screen
    const uint8_t dataMsg[] = {
        SSD1306_DATA_START,         // start data
        ssd1306_vram[page][x]       //   VRAM page
    };
    if (twi_write(SSD1306_I2C_ADDRESS, dataMsg, sizeof dataMsg))       return 4;

    // return successful
    return 0;
}

void ssd1306_clear()
{
    const uint8_t configMsg[] = {
	    SSD1306_CMD_START,          // start commands
	    SSD1306_SETPAGERANGE,       // set page range:
	    SSD1306_PAGE_START,                       //   y / 8
	    SSD1306_PAGE_STOP,                       //   y / 8
	    SSD1306_SETCOLRANGE,        // set column range:
	    SSD1306_COL_START,                          //   x
	    SSD1306_COL_STOP                           //   x		
    };
    if (twi_write(SSD1306_I2C_ADDRESS, configMsg, sizeof configMsg))   return 3;

	twi_repeat(SSD1306_I2C_ADDRESS, SSD1306_DATA_START, 0, sizeof ssd1306_vram);
}

/***************************************************************************************************
 void ssd1306_displayChar( char ch)
 ****************************************************************************************************
 * I/P Arguments: ASCII value of the char to be displayed.
 * Return value    : none
 * description  : This function sends a character to be displayed on LCD.
 Any valid ascii value can be passed to display respective character
 ****************************************************************************************************/
void ssd1306_displayChar(uint8_t ch)
{
    uint8_t dat,i=0;
    
    if(((ssd1306_cursorPos+FONT_SIZE)>=128) || (ch=='\n'))
    {
        /* If the cursor has reached to end of line on page1
         OR NewLine command is issued Then Move the cursor to next line */
        ssd1306_goToNextLine();
    }
    if(ch!='\n') /* TODO */
    {
		
		uint8_t data[FONT_SIZE+2];
		data[0] = SSD1306_DATA_START;
		
        ch = ch-0x20; // As the lookup table starts from Space(0x20)
        
        while(1)
        {
            dat= OledFontTable[ch][i]; /* Get the data to be displayed for LookUptable*/
            data[i+1] = dat;
            
			
            ssd1306_cursorPos++;
            
            i++;
            
            if(i==FONT_SIZE) /* Exit the loop if End of char is encountered */
            {
				data[i+1] = 0;
				
                ssd1306_cursorPos++;
                break;
            }
        }
		twi_write(SSD1306_I2C_ADDRESS, data, sizeof data);
    }
}




/***************************************************************************************************
 void OLED_DisplayString(char *ptr_stringPointer_u8)
 ****************************************************************************************************
 * I/P Arguments: String(Address of the string) to be displayed.
 * Return value    : none
 * description  :
 This function is used to display the ASCII string on the lcd.
 1.The ptr_stringPointer_u8 points to the first char of the string
 and traverses till the end(NULL CHAR)and displays a char each time.
 ****************************************************************************************************/

void ssd1306_displayString(uint8_t *ptr)
{
    while(*ptr)
        ssd1306_displayChar(*ptr++);
}





/***************************************************************************************************
 void OLED_GoToLine(uint8_t v_lineNumber_u8)
 ****************************************************************************************************
 * I/P Arguments: uint8_t: Line number(0-7).
 * Return value    : none
 * description  :This function moves the Cursor to beginning of the specified line.
 If the requested line number is out of range, it will not move the cursor.
 Note: The line numbers run from 0 to 7
 ****************************************************************************************************/

void  ssd1306_goToLine(uint8_t lineNumber)
{
    if(lineNumber<8)
    {   /* If the line number is within range
         then move it to specified line and keep track*/
        ssd1306_lineNum = lineNumber;
        ssd1306_setCursor(ssd1306_lineNum,0);
    }
}







/***************************************************************************************************
 void  OLED_GoToNextLine()
 ****************************************************************************************************
 * I/P Arguments: none
 * Return value    : none
 * description  :This function moves the Cursor to beginning of the next line.
 If the cursor is on last line and NextLine command is issued then
 it will move the cursor to first line.
 ****************************************************************************************************/
void  ssd1306_goToNextLine()
{
    /*Increment the current line number.
     In case it exceeds the limit, rool it back to first line */
    ssd1306_lineNum++;
    ssd1306_lineNum = ssd1306_lineNum&0x07;
    ssd1306_setCursor(ssd1306_lineNum,0); /* Finally move it to next line */
}







/***************************************************************************************************
 void OLED_SetCursor(char v_lineNumber_u8,char v_charNumber_u8)
 ****************************************************************************************************
 * I/P Arguments: char row,char col
 row -> line number(line1=1, line2=2),
 For 2line LCD the I/P argument should be either 1 or 2.
 col -> char number.
 For 16-char LCD the I/P argument should be between 0-15.
 * Return value    : none
 * description  :This function moves the Cursor to specified position
 Note:If the Input(Line/Char number) are out of range
 then no action will be taken
 ****************************************************************************************************/
void ssd1306_setCursor(uint8_t lineNumber,uint8_t cursorPosition)
{
    /* Move the Cursor to specified position only if it is in range */
    if((lineNumber <= C_SSD1306LastLine_U8) && (cursorPosition <= 127))
    {
        ssd1306_lineNum=lineNumber;   /* Save the specified line number */
        ssd1306_cursorPos=cursorPosition; /* Save the specified cursor position */
        
		uint8_t commands[] = {
			SSD1306_CMD_START,
			SSD1306_SETCOLRANGE,
			cursorPosition, 
			127,
			SSD1306_SETPAGERANGE,
			lineNumber,
			7
		};
		
		twi_write(SSD1306_I2C_ADDRESS, commands, sizeof commands);
		
    }
}



/***************************************************************************************************
 void OLED_DisplayNumber(uint8_t v_numericSystem_u8, uint32_t v_number_u32, uint8_t v_numOfDigitsToDisplay_u8)
 ****************************************************************************************************
 * Function name:  OLED_DisplayNumber()
 * I/P Arguments:
 uint8_t :  specifies type of number C_BINARY_U8(2),C_DECIMAL_U8(10), C_HEX_U8(16)
 uint32_t: Number to be displayed on the LCD.
 uint8_t : Number of digits to be displayed
 * Return value    : none
 * description  :This function is used to display a max of 10digit decimal/Hex number OR specified
 number of bits for binary number.
 1st parameter specifies type of number C_BINARY_U8(2),C_DECIMAL_U8(10), C_HEX_U8(16)
 3rd parameter specifies the number of digits from the right side to be displayed
 The output for the input combinations is as below
 Binary:     1.(2,10,4) then 4-LSB will be displayed ie. 1010
 2.(C_BINARY_U8,10,8) then 8-LSB will be displayed ie. 00001010
 3.(C_BINARY_U8,10,2) then 2-LSB will be displayed ie. 10
 Decimal:    4.(10,12345,4) then 4-digits ie. 2345 will be displayed
 5.(C_DECIMAL_U8,12345,6) then 6-digits ie. 012345 will be displayed
 6.(10,12345,C_DisplayDefaultDigits_U8) then 12345 will be displayed.\
 Hex:        7.(16,0x12AB,3) then 3-digits ie. 2AB will be displayed
 8.(C_HEX_U8,0x12AB,6) then 6-digits ie. 0012AB will be displayed
 9.(C_HEX_U8,0x12AB,C_DisplayDefaultDigits_U8) then 12AB will be displayed.
 ****************************************************************************************************/

void ssd1306_displayNumber(uint8_t v_numericSystem_u8, uint32_t v_number_u32, uint8_t v_numOfDigitsToDisplay_u8)
{
    uint8_t i=0,a[10];
    
    if(C_BINARY_U8 == v_numericSystem_u8)
    {
        while(v_numOfDigitsToDisplay_u8!=0)
        {
            /* Start Extracting the bits from the specified bit positions.
             Get the Acsii values of the bits and display */
            i = util_GetBitStatus(v_number_u32,(v_numOfDigitsToDisplay_u8-1));
            ssd1306_displayChar(util_Dec2Ascii(i));
            v_numOfDigitsToDisplay_u8--;
        }
    }
    else if(v_number_u32==0)
    {
        /* If the number is zero then update the array with the same for displaying */
        for(i=0;((i<v_numOfDigitsToDisplay_u8) && (i<C_MaxDigitsToDisplay_U8));i++)
            ssd1306_displayChar('0');
    }
    else
    {
        for(i=0;i<v_numOfDigitsToDisplay_u8;i++)
        {
            /* Continue extracting the digits from right side
             till the Specified v_numOfDigitsToDisplay_u8 */
            if(v_number_u32!=0)
            {
                /* Extract the digits from the number till it becomes zero.
                 First get the remainder and divide the number by TypeOfNum(10-Dec, 16-Hex) each time.
                 example for Decimal number:
                 If v_number_u32 = 123 then extracted remainder will be 3 and number will be 12.
                 The process continues till it becomes zero or max digits reached*/
                a[i]=util_GetMod32(v_number_u32,v_numericSystem_u8);
                v_number_u32=v_number_u32/v_numericSystem_u8;
            }
            else if( (v_numOfDigitsToDisplay_u8 == C_DisplayDefaultDigits_U8) ||
                    (v_numOfDigitsToDisplay_u8 > C_MaxDigitsToDisplay_U8))
            {
                /* Stop the iteration if the Max number of digits are reached or
                 the user expects exact(Default) digits in the number to be displayed */
                break;
            }
            else
            {
                /* In case user expects more digits to be displayed than the actual digits in number,
                 then update the remaining digits with zero.
                 Ex: v_num_u32 is 123 and user wants five digits then 00123 has to be displayed */
                a[i]=0;
            }
        }
        
        while(i!=0)
        {
            /* Finally get the ascii values of the digits and display*/
            ssd1306_displayChar(util_Hex2Ascii(a[i-1]));
            i--;
        }
    }
}









/*************************************************************************************************
 void  OLED_DisplayFloatNumber(double v_floatNum_f32)
 **************************************************************************************************
 * Function name:  OLED_DisplayFloatNumber()
 * I/P Arguments: float: float Number to be displayed on the LCD.
 * Return value    : none
 * description  :This function is used to display a floating point number
 It supports 6digits of precision.
 Note: Float will be disabled by default as it takes huge controller resources
 It can be enabled by changing value of Enable_OLED_DisplayFloatNumber to 1 in lcd.h
 ***************************************************************************************************/

void ssd1306_displayFloatNumber(double v_floatNum_f32)
{
    uint32_t v_temp_u32;
    /* Dirty hack to support the floating point by extracting the integer and fractional part.
     1.Type cast the number to int to get the integer part.
     2.Display the extracted integer part followed by a decimal point(.)
     3.Later the integer part is made zero by subtracting with the extracted integer value.
     4.Finally the fractional part is multiplied by 100000 to support 6-digit precision */
    
    v_temp_u32 = (uint32_t) v_floatNum_f32;
    ssd1306_displayNumber(C_DECIMAL_U8,v_temp_u32,C_DisplayDefaultDigits_U8);
    
    ssd1306_displayChar('.');
    
    v_floatNum_f32 = v_floatNum_f32 - v_temp_u32;
    v_temp_u32 = v_floatNum_f32 * 1000000;
    ssd1306_displayNumber(C_DECIMAL_U8,v_temp_u32,C_DisplayDefaultDigits_U8);
}







/*************************************************************************************************
 void OLED_Printf(const char *argList, ...)
 **************************************************************************************************
 * Function name:  OLED_Printf()
 * I/P Arguments: variable length arguments similar to printf
 * Return value    : none
 * description  :This function is similar to printf function in C.
 It takes the arguments with specified format and prints accordingly
 The supported format specifiers are as below.
 1. %c: character
 2. %d: signed 16-bit number
 3. %D: signed 32-bit number
 4. %u: unsigned 16-bit number
 5. %U: unsigned 32-bit number
 6. %b: 16-bit binary number
 7. %B: 32-bit binary number
 8. %f: Float number
 9. %x: 16-bit hexadecimal number
 10. %X: 32-bit hexadecimal number
 11. %s: String
 Extra feature is available to specify the number of digits to be displayed using printf.
 ex: %4d: will display the lower four digits of the decimal number.
 %12b: will display the 12-LSB of the number
 %d: Will display the exact digits of the number
 #####: In case of printing the 8-bit variables, it is recommended to type cast and promote them to uint16_t.
 uint8_t v_Num_u8;
 OLED_Printf("num1:%u",(uint16_t)v_Num_u8);
 **************************************************************************************************/

void ssd1306__printf(const char *argList, ...)
{
    const char *ptr;
    va_list argp;
    sint16_t v_num_s16;
    sint32_t v_num_s32;
    uint16_t v_num_u16;
    uint32_t v_num_u32;
    char *str;
    char  ch;
    uint8_t v_numOfDigitsToDisp_u8;

    double v_floatNum_f32;

    
    va_start(argp, argList);
    
    /* Loop through the list to extract all the input arguments */
    for(ptr = argList; *ptr != '\0'; ptr++)
    {
        
        ch= *ptr;
        if(ch == '%')         /*Check for '%' as there will be format specifier after it */
        {
            ptr++;
            ch = *ptr;
            if((ch>=0x30) && (ch<=0x39))
            {
                v_numOfDigitsToDisp_u8 = 0;
                while((ch>=0x30) && (ch<=0x39))
                {
                    v_numOfDigitsToDisp_u8 = (v_numOfDigitsToDisp_u8 * 10) + (ch-0x30);
                    ptr++;
                    ch = *ptr;
                }
            }
            else
            {
                v_numOfDigitsToDisp_u8 = C_MaxDigitsToDisplayUsingPrintf_U8;
            }
            
            
            switch(ch)       /* Decode the type of the argument */
            {
                case 'C':
                case 'c':     /* Argument type is of char, hence read char data from the argp */
                    ch = va_arg(argp, int);
                    ssd1306_displayChar(ch);
                    break;
                    
                case 'd':    /* Argument type is of signed integer, hence read 16bit data from the argp */
                    v_num_s16 = va_arg(argp, int);
                    if(v_num_s16<0)
                    { /* If the number is -ve then display the 2's complement along with '-' sign */
                        v_num_s16 = -v_num_s16;
                        ssd1306_displayChar('-');
                    }
                    ssd1306_displayNumber(C_DECIMAL_U8,v_num_s16,v_numOfDigitsToDisp_u8);
                    break;
                    
                case 'D':    /* Argument type is of integer, hence read 16bit data from the argp */
                    v_num_s32 = va_arg(argp, sint32_t);
                    if(v_num_s32<0)
                    { /* If the number is -ve then display the 2's complement along with '-' sign */
                        v_num_s32 = -v_num_s32;
                        ssd1306_displayChar('-');
                    }
                    ssd1306_displayNumber(C_DECIMAL_U8,v_num_s32,v_numOfDigitsToDisp_u8);
                    break;
                    
                case 'u':    /* Argument type is of unsigned integer, hence read 16bit unsigned data */
                    v_num_u16 = va_arg(argp, int);
                    ssd1306_displayNumber(C_DECIMAL_U8,v_num_u16,v_numOfDigitsToDisp_u8);
                    break;
                    
                case 'U':    /* Argument type is of integer, hence read 32bit unsigend data */
                    v_num_u32 = va_arg(argp, uint32_t);
                    ssd1306_displayNumber(C_DECIMAL_U8,v_num_u32,v_numOfDigitsToDisp_u8);
                    break;
                    
                case 'x':  /* Argument type is of hex, hence hexadecimal data from the argp */
                    v_num_u16 = va_arg(argp, int);
                    ssd1306_displayNumber(C_HEX_U8,v_num_u16,v_numOfDigitsToDisp_u8);
                    break;
                    
                case 'X':  /* Argument type is of hex, hence hexadecimal data from the argp */
                    v_num_u32 = va_arg(argp, uint32_t);
                    ssd1306_displayNumber(C_HEX_U8,v_num_u32,v_numOfDigitsToDisp_u8);
                    break;
                    
                    
                case 'b':  /* Argument type is of binary,Read int and convert to binary */
                    v_num_u16 = va_arg(argp, int);
                    if(v_numOfDigitsToDisp_u8 == C_MaxDigitsToDisplayUsingPrintf_U8)
                        v_numOfDigitsToDisp_u8 = 16;
                    ssd1306_displayNumber(C_BINARY_U8,v_num_u16,v_numOfDigitsToDisp_u8);
                    break;
                    
                case 'B':  /* Argument type is of binary,Read int and convert to binary */
                    v_num_u32 = va_arg(argp, uint32_t);
                    if(v_numOfDigitsToDisp_u8 == C_MaxDigitsToDisplayUsingPrintf_U8)
                        v_numOfDigitsToDisp_u8 = 16;
                    ssd1306_displayNumber(C_BINARY_U8,v_num_u32,v_numOfDigitsToDisp_u8);
                    break;
                    
                    
                case 'F':
                case 'f': /* Argument type is of float, hence read double data from the argp */

                    v_floatNum_f32 = va_arg(argp, double);
                    ssd1306_displayFloatNumber(v_floatNum_f32);

                    break;
                    
                    
                case 'S':
                case 's': /* Argument type is of string, hence get the pointer to sting passed */
                    str = va_arg(argp, char *);
                    ssd1306_displayString(str);
                    break;
                    
                case '%':
                    ssd1306_displayChar('%');
                    break;
            }
        }
        else
        {
            /* As '%' is not detected display/transmit the char passed */
            ssd1306_displayChar(ch);
        }
    }
    
    va_end(argp);
}




/**************************************************************
 * Dirty Rectangle Animation **********************************
 **************************************************************/
static DirtyRectangleBuffer dirtyRectBuff;

/*
 * ! write an (x,y) coordinate pair to the Dirty Rectangle buffer
 * !
 * ! \param x: x coordinate to write to buffer
 * ! \param y: y coordinate to write to buffer
 * !
 * ! \return 0 if successful, 1 if buffer was already full
 */
uint16_t dirtyRect_write(uint8_t x, uint8_t y) {
    // check if buffer is full
    if (dirtyRectBuff.length == (1 << DIRTY_RECT_BUFFER_POWER))
        return 1;
    // else write values, increment writeIndex & length
    dirtyRectBuff.coordinates[0][dirtyRectBuff.writeIndex] = x;
    dirtyRectBuff.coordinates[1][dirtyRectBuff.writeIndex] = y;
    dirtyRectBuff.writeIndex = (dirtyRectBuff.writeIndex + 1) & ((1 << DIRTY_RECT_BUFFER_POWER) - 1);
    dirtyRectBuff.length++;
    return 0;
}

/*
 * ! read an (x,y) coordinate pair from the Dirty Rectangle buffer
 * !
 * ! \param *x: reference to x value to store Dirty Rectangle buffer value
 * ! \param *y: reference to y value to store Dirty Rectangle buffer value
 * !
 * ! \return 0 if successful, 1 if buffer empty
 */
uint16_t dirtyRect_read(uint8_t *x, uint8_t *y) {
    // check if buffer is empty
    if (!dirtyRectBuff.length)
        return 1;
    //else read values, increment readIndex, decrement length
    *x = dirtyRectBuff.coordinates[0][dirtyRectBuff.readIndex];
    *y = dirtyRectBuff.coordinates[1][dirtyRectBuff.readIndex];
    dirtyRectBuff.readIndex = (dirtyRectBuff.readIndex + 1) & ((1 << DIRTY_RECT_BUFFER_POWER) - 1);
    dirtyRectBuff.length--;
    return 0;
}



/**************************************************************
 *  Display Functionality *************************************
 **************************************************************/
static DisplayRegionList dispRegionList;


/*
 * ! clear last frame from VRAM and update Display Region List;
 * ! must be called at the start of each frame to draw
 */
void display_frameStart(void) {
    // clear drawings from last frame (Dirty Rectangle Animation)
    uint8_t x=0,
            y=0;
    while(!dirtyRect_read(&x, &y)) {
        // determine column range
        uint8_t colStop = x + 7;
        if (colStop >= SSD1306_COLUMNS) {
            colStop = SSD1306_COLUMNS - 1;
        }

        // determine page range
        uint8_t pageStart = y >> 3, // y / 8 = starting page
                pageStop = y >> 3;  // y / 8 = stopping page, unless..
        if (y & 0x07)               // if y is not an integer multiple of 8,
            pageStop++;             //   two pages must be updated

        // add (page,col) coordinates to display region list
        dispRegionList.coordinates[0][dispRegionList.length] = pageStart;
        dispRegionList.coordinates[1][dispRegionList.length] = x;
        dispRegionList.length++;
        if (pageStop != pageStart) {
            if(pageStop <= SSD1306_PAGE_STOP) {
                dispRegionList.coordinates[0][dispRegionList.length] = pageStop;
                dispRegionList.coordinates[1][dispRegionList.length] = x;
                dispRegionList.length++;
            }
        }

        // update VRAM
        unsigned int i = colStop - x + 1,       // counter for iterating
                     pageOffset = y & 0x07;     // offset from bottom of page
        while (i!=0) {
            i--;                                        // decrement counter
            uint8_t lowerPage = 0xFF << pageOffset;     // move box 'up'
            ssd1306_vram[pageStart][x+i] &= ~lowerPage; // clear from VRAM
            // clear second page if necessary and valid
            if ((pageStop != pageStart) && (pageStop <= SSD1306_PAGE_STOP)) {
                uint8_t upperPage = 0xFF >> (8-pageOffset); // move box 'down'
                ssd1306_vram[pageStop][x+i] &= ~upperPage;  // clear from VRAM
            }
        }
    }
}


/*
 * ! Draw an 8x8 sprite at location (x,y) in VRAM
 * !
 * ! \param x: x coordinate of bottom left pixel of sprite [0:SSD1306_COLUMNS-1]
 * ! \param y: y coordinate of bottom left pixel of sprite [0:SSD1306_ROWS-1]
 * ! \param *sprite: pointer to 8x8 sprite data (const uint8_t[8])
 * !
 * ! \return 0: successful, else error:
 * !         1: x value out of range
 * !         2: y value out of range
 * !         3: dirty rectangle animation buffer was full
 * !
 * ! Draws an 8x8 sprite to on-chip VRAM
 */
uint16_t display_drawSprite(uint16_t x, uint16_t y, const uint8_t *const sprite) {
    // ensure pixel location is valid
    if (x >= SSD1306_COLUMNS)   return 1;
    if (y >= SSD1306_ROWS)      return 2;

    // add (x,y) coordinate to dirty rectangle animation buffer
    if (dirtyRect_write(x, y))  return 3;

    // determine column range: [x:x+7]
    uint8_t colStop = x + 7;
    if (colStop >= SSD1306_COLUMNS) {
        colStop = SSD1306_COLUMNS - 1;
    }

    // determine page range
    uint8_t pageStart = y >> 3, // y / 8 = starting page
            pageStop = y >> 3;  // y / 8 = stopping page, unless..
    if (y & 0x07)               // if y is not an integer multiple of 8,
        pageStop++;             //   two pages must be updated

    // add (page,col) coordinates to display region list
    dispRegionList.coordinates[0][dispRegionList.length] = pageStart;
    dispRegionList.coordinates[1][dispRegionList.length] = x;
    dispRegionList.length++;
    if (pageStop != pageStart) {
        if (pageStop <= SSD1306_PAGE_STOP) {
            dispRegionList.coordinates[0][dispRegionList.length] = pageStop;
            dispRegionList.coordinates[1][dispRegionList.length] = x;
            dispRegionList.length++;
        }
    }

    // update VRAM
    unsigned int i = colStop - x + 1,       // counter for iterating
                 pageOffset = y & 0x07;     // offset from bottom of page
    while (i!=0) {
        i--;                                              // decrement counter
        uint8_t lowerPage = sprite[i] << pageOffset;      // move sprite 'up'
        ssd1306_vram[pageStart][x+i] |= lowerPage;        // OR into VRAM
        //if necessary, update second page
        if ((pageStart != pageStop) && (pageStop <= SSD1306_PAGE_STOP)) {
            uint8_t upperPage = sprite[i] >> (8-pageOffset);    // move sprite 'down'
            ssd1306_vram[pageStop][x+i] |= upperPage;           // OR into VRAM
        }
    }

    // return successful
    return 0;
}


/*
 * ! Draw an 8x8 sprite at location (x,y) in VRAM
 * !
 * ! \param x: x coordinate of bottom left pixel of sprite [0:SSD1306_COLUMNS-1]
 * ! \param y: y coordinate of bottom left pixel of sprite [0:SSD1306_ROWS-1]
 * ! \param *sprite: pointer to 8x8 sprite data (const uint8_t[8])
 * !
 * ! \return 0: successful, else error:
 * !         1: I2C error during configuration
 * !         2: I2C error during data transmission
 * !         3: dirty rectangle animation buffer was full
 * !
 * ! Draws an 8x8 sprite to on-chip VRAM
 */
uint16_t display_drawFrame(void) {
    // cocktail sort local variables
    uint16_t start = 0,
             end = dispRegionList.length - 1,
             swapped = 1,
             i = 0;

    // sort display region list by column (coordinates[1][:])
    while(swapped) {
        swapped = 0;
        //move forward through array
        for(i=start; i<end; i++) {
            if (dispRegionList.coordinates[1][i] > dispRegionList.coordinates[1][i+1]) {
                uint8_t tempPage = dispRegionList.coordinates[0][i],
                        tempCol  = dispRegionList.coordinates[1][i];
                dispRegionList.coordinates[0][i] = dispRegionList.coordinates[0][i+1];
                dispRegionList.coordinates[1][i] = dispRegionList.coordinates[1][i+1];
                dispRegionList.coordinates[0][i+1] = tempPage;
                dispRegionList.coordinates[1][i+1] = tempCol;
                swapped = 1;
            }
        }
        if (!swapped) break;
        swapped = 0;
        end--;
        // move backwards through array
        for(i=end; i>start; i--) {
            if (dispRegionList.coordinates[1][i-1] > dispRegionList.coordinates[1][i]) {
                uint8_t tempPage = dispRegionList.coordinates[0][i],
                        tempCol  = dispRegionList.coordinates[1][i];
                dispRegionList.coordinates[0][i] = dispRegionList.coordinates[0][i-1];
                dispRegionList.coordinates[1][i] = dispRegionList.coordinates[1][i-1];
                dispRegionList.coordinates[0][i-1] = tempPage;
                dispRegionList.coordinates[1][i-1] = tempCol;
                swapped = 1;
            }
        }
        start++;
    }

    // sort display region list by page (coordinates[0][:])
    start = 0;
    end = dispRegionList.length-1;
    swapped = 1;
    while(swapped) {
        swapped = 0;
        //move forward through array
        for(i=start; i<end; i++) {
            if (dispRegionList.coordinates[0][i] > dispRegionList.coordinates[0][i+1]) {
                uint8_t tempPage = dispRegionList.coordinates[0][i],
                        tempCol  = dispRegionList.coordinates[1][i];
                dispRegionList.coordinates[0][i] = dispRegionList.coordinates[0][i+1];
                dispRegionList.coordinates[1][i] = dispRegionList.coordinates[1][i+1];
                dispRegionList.coordinates[0][i+1] = tempPage;
                dispRegionList.coordinates[1][i+1] = tempCol;
                swapped = 1;
            }
        }
        if (!swapped) break;
        swapped = 0;
        end--;
        // move backwards through array
        for(i=end; i>start; i--) {
            if (dispRegionList.coordinates[0][i-1] > dispRegionList.coordinates[0][i]) {
                uint8_t tempPage = dispRegionList.coordinates[0][i],
                        tempCol  = dispRegionList.coordinates[1][i];
                dispRegionList.coordinates[0][i] = dispRegionList.coordinates[0][i-1];
                dispRegionList.coordinates[1][i] = dispRegionList.coordinates[1][i-1];
                dispRegionList.coordinates[0][i-1] = tempPage;
                dispRegionList.coordinates[1][i-1] = tempCol;
                swapped = 1;
            }
        }
        start++;
    }

    // write merged regions to SSD1306
    i = 0;
    while(i < dispRegionList.length) {
        // find page and column ranges
        uint8_t page = dispRegionList.coordinates[0][i];
        uint8_t colStart = dispRegionList.coordinates[1][i];
        uint8_t colStop = colStart + 7;

        // attempt to merge column range to update
        while((i < dispRegionList.length - 1) &&                    // as long as there's more
              (dispRegionList.coordinates[0][i+1] == page) &&       // & on the same page
              (dispRegionList.coordinates[1][i+1] <= colStop)) {    // & within column range
                    i++;                                                // increment counter
                    colStop = dispRegionList.coordinates[1][i] + 7;     // update colStop
        }

        // ensure colStop is valid
        if (colStop >= SSD1306_COLUMNS) colStop = SSD1306_COLUMNS-1;

        // send configuration message
        const uint8_t configMsg[] = {
            SSD1306_CMD_START,          // start commands
            SSD1306_SETPAGERANGE,       // set page range:
            page,                       //
            page,                       //
            SSD1306_SETCOLRANGE,        // set column range:
            colStart,                   //
            colStop                     //
        };
        if (twi_write(SSD1306_I2C_ADDRESS, configMsg, sizeof configMsg))   return 1;

        // draw updated VRAM to screen
        uint8_t dataMsg[129] = {    // message can be a max of 129 bytes
            SSD1306_DATA_START      // start data
        };
        unsigned int j = colStop - colStart + 1;    // local counter to
        while (j != 0) {                            // copy VRAM into dataMsg
            j--;
            dataMsg[j+1] = ssd1306_vram[page][colStart+j];
        }
        if (twi_write(SSD1306_I2C_ADDRESS, dataMsg, colStop-colStart+2))  return 2;

        // update loop counter
        i++;
    }

    // clear display regions list for next frame
    dispRegionList.length = 0;

    // return successful
    return 0;
}


/*
 * ! Clear the SSD1306 OLED Display
 * !
 * ! \return logical OR of all errors encountered;
 * !         0 if succesful
 * !
 * ! Clears the entire display by blanking the screen in four consecutive
 * ! frame draws.
 */
uint16_t display_clearScreen(void) {
    uint16_t r = 0,     // display row
             c = 0,     // display column
             error = 0; // returned errors
    // blank sprite for drawings
    const uint8_t blank[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (r = 0; r < SSD1306_COLUMNS; r += 8) {
        // clear each row in a frame
        display_frameStart();
        for (c = 0; c < SSD1306_ROWS; c += 8) {
            error |= display_drawSprite(r, c, blank);
        }
        error |= display_drawFrame();
    }
    // return accumulated errors
    return error;
}
