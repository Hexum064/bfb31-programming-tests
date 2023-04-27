/*
 * xmega16e5-xcl-ws2812-test.c
 *
 * Created: 2023-04-26 18:04:34
 * Author : Branden
 */ 

#define F_CPU 32000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ws2812drv.h"

ws2812drv_led_t leds[50];

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

uint8_t offset = 0;

void load_leds()
{
	for (uint8_t i = 0; i < 50; i++)
	{
		leds[i].r = 0;
		leds[i].g = 0;
		leds[i].b = 0;
		
		switch((i + offset)%3)
		{
			case 0:
				leds[i].r = 64;
				break;
			case 1:
				leds[i].g = 64;
				break;
			case 2:
				leds[i].b = 64;
				break;
		}
	}
	offset++;
}

int main(void)
{
	cli();
	clk_init();
	interrupt_init();
	sei();
	ws2812drv_init();
	PORTA.DIRSET = PIN3_bm;
    /* Replace with your application code */
    while (1) 
    {
		PORTA.OUTTGL = PIN3_bm;
		load_leds();
		ws2812drv_start_transfer(leds, 50);
		_delay_ms(100);
    }
}

