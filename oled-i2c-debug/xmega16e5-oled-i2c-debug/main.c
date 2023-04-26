/*
 *
 *
 * Created: 2023-03-01 19:35:41
 * Author : Branden
 */ 

#define F_CPU 32000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "SSD1306.h"
#include "i2c.h"

void clk_init()
{
	CCP = CCP_IOREG_gc;
	OSC_CTRL = OSC_RC32MEN_bm;
	
	while(!(OSC_STATUS & OSC_RC32MRDY_bm)){};
	
	CCP = CCP_IOREG_gc;
	CLK_CTRL = CLK_SCLKSEL_RC32M_gc;
	
	CCP = CCP_IOREG_gc;
	CLK_PSCTRL = 0;
	
}

void interrupt_init()
{
	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
}

int main(void)
{
	
 	cli();
	clk_init();
 	interrupt_init();

 	twi_init();
	
 	sei();	

 	ssd1306_init();

 	ssd1306_clear();
 	ssd1306_setCursor(0, 0);
 	ssd1306__printf("Hello number %d.",1);
 	PORTA.DIRSET = PIN3_bm;
	uint16_t x = 0;
	
    while (1) 
    {
	    //ssd1306_clear();
	    ssd1306_setCursor(0, 0);
	    ssd1306__printf("Hello number %d.",x++);
		PORTA.OUTTGL = PIN3_bm;
		//_delay_ms(100);
		
    }
}

