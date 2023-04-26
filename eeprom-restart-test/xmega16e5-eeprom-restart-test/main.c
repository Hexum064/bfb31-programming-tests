/*
 * xmega16e5-eeprom-restart-test.c
 *
 * Created: 2023-04-23 19:04:36
 * Author : Branden
 */ 

 
#define F_CPU 32000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "SSD1306.h"
#include "i2c.h"

#define EXP_ADDR 0x20
#define EXP_WRITE 0x00
#define EXP_READ 0x01
#define EXP_REG_CTRL0 0x06
#define EXP_REG_CTRL1 0x07

#define MAX_MODE 7

volatile uint8_t start_mode = 0;

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

void oled_init()
{
	ssd1306_init();

	ssd1306_clear();
	ssd1306_setCursor(0, 0);
	ssd1306__printf("Started");
}


uint8_t get_mode_from_eeprom()
{
	return eeprom_read_byte((uint8_t*)1);
}

void write_mode_to_eeprom(uint8_t value)
{
	eeprom_write_byte((uint8_t *)1, value);
}

void enable_wdt()
{
	CCP = CCP_IOREG_gc;
	WDT_CTRL = WDT_ENABLE_bm | WDT_CEN_bm;
}

int main(void)
{
	
	cli();
	clk_init();
	interrupt_init();
	twi_init();
	sei();
	oled_init();

	PORTA.DIRCLR = PIN4_bm;
	PORTA.PIN4CTRL = PORT_OPC_PULLUP_gc;
	
	while(!(PORTA.IN & PIN4_bm));
	
	start_mode = get_mode_from_eeprom();
	ssd1306_setCursor(1, 0);
	ssd1306__printf("Mode: %d.", start_mode);

	
	while (1)
	{
		if (!(PORTA.IN & PIN4_bm))
		{
			_delay_ms(100);
			start_mode++;
			
			if (start_mode > MAX_MODE)
			{
				start_mode = 0;
			}
			
			write_mode_to_eeprom(start_mode);
			while(!(PORTA.IN & PIN4_bm));
			
			enable_wdt();	
		}
		
	}
}

