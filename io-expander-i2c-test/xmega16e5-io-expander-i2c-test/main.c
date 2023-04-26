/*
 * xmega16e5-io-expander-i2c-test.c
 *
 * Created: 2023-04-22 20:43:40
 * Author : Branden
 */ 
#define F_CPU 32000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "SSD1306.h"
#include "i2c.h"

#define EXP_ADDR 0x20
#define EXP_WRITE 0x00
#define EXP_READ 0x01
#define EXP_REG_CTRL0 0x06
#define EXP_REG_CTRL1 0x07


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

void expander_init()
{
	//Set up port0 and port1 as input
	uint8_t addr = EXP_ADDR;
	uint8_t cmd[] = {
		EXP_REG_CTRL0,
		0xff,
		0xff
	};
	twi_write(addr, cmd, 3);
	
}
uint8_t port_val;
uint8_t expander_read_port(uint8_t portAddr)
{
	
	uint8_t addr = EXP_ADDR;
	
	uint8_t cmd[] = {
		portAddr
	};
	
	twi_write(addr, cmd, sizeof cmd);
	
	twi_read(addr, &port_val, 1);
	
	return port_val;
	
}

uint16_t expander_read_ports()
{
	uint16_t port_data;
	
	port_data = expander_read_port(0x00) + (expander_read_port(0x01) << 8);
	return port_data;
	
}

void d_to_b(uint16_t val, char * bits)
{
	uint8_t i = 0;
	
	while (i < 16)
	{
		bits[i] = '0';
		if (val & (0x8000 >> i))
		{
			bits[i] = '1';
		}
		i++;
	}
	
	
}

int main(void)
{
	
	cli();
	clk_init();
	interrupt_init();
	twi_init();
	sei();
	PORTA.DIRSET = PIN3_bm;
	PORTA.DIRCLR = PIN2_bm;
	PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc;
	oled_init();
 	expander_init();



	uint16_t exp_val = 0;
	//ssd1306_displayString("Go");
// 	ssd1306_setCursor(0,9);
	//exp_val = expander_read_ports();
	//ssd1306_displayString("expander read");
	ssd1306_setCursor(1,0);
	ssd1306__printf("expander read");
	PORTA.OUTSET = PIN3_bm;
	
	char bits[17] = {'0'};
	bits[16] = '\0';
	while (1)
	{

		exp_val = expander_read_ports();
		exp_val &= 0xfffe;
		exp_val |= ((PORTA.IN & PIN2_bm) >> 2);
		
		if (exp_val != 0xffff)
		{
			d_to_b(exp_val, bits);	
		}
		
		ssd1306_setCursor(2,0);
		ssd1306_displayString(bits);
	
 		//_delay_ms(500);
		PORTA.OUTTGL = PIN3_bm;
	}
}

