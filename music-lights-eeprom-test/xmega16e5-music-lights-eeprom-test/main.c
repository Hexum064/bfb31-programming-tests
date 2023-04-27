/*
 * xmega16e5-music-lights-eeprom-test.c
 *
 * Created: 2023-04-26 17:24:01
 * Author : Branden
 */ 

#define F_CPU 32000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "SSD1306.h"
#include "i2c.h"
#include "ws2812drv.h"

#define EXP_ADDR 0x20
#define EXP_WRITE 0x00
#define EXP_READ 0x01
#define EXP_REG_CTRL0 0x06
#define EXP_REG_CTRL1 0x07

#define INTRO_TREBLE_NOTE_COUNT 31
#define MAIN_TREBLE_NOTE_COUNT 212

#define INTRO_BASS_NOTE_COUNT 2
#define MAIN_BASS_NOTE_COUNT 17

#define STARTING_NOTE_INDEX 7

#define MAX_MODE 1

volatile uint8_t start_mode = 0;

volatile ws2812drv_led_t leds[50];

uint16_t exp_decay_vals[] = {4095, 4000, 3800, 3500, 3100, 2600, 2000, 1300, 500, 400, 300, 200, 100};
	
//G6# to F1
uint16_t noteClocks[] = {1204,1276,1352,1432,1516,1608,1704,1804,1912,2024,2144,2272,2408,2552,2704,2864,3032,3216,3404,3608,3824,4048,4292,4544,4816,5104,5404,5728,6068,6428,6812,7216,7644,8100,8580,9092,9632,10204,10812,11456,12136,12856,13620,14432,15288,16200,17160,18180,19264,20408,21620,22908,24268,25712,27240,28860,30576,32392,34324,36364,38528,40816,43244,45820};
uint8_t nyanIntroTreble[] = {0x12,0x11,0xf,0x0,0xa,0x11,0x12,0x11,0xf,0xa,0x6,0x5,0x6,0xb,0xa,0x0,0xf,0x0,0x12,0x11,0xf,0x0,0x4a,0x8,0xb,0xa,0x8,0x5,0x6,0x5,0x8};
uint8_t nyanIntroExtTreble[] = {0x0,0x0,0x0,0x0};

uint8_t nyanMainTreble[] = {0x4f,0x4d,0x15,0x52,0x14,0x13,0x14,0x56,0x56,0x54,0x53,0x13,0x14,0x16,0x14,0x12,0xf,0xd,0x12,0xf,0x14,0x12,0x16,0x14,0x16,0x52,0x4f,0xd,0x12,0xf,0x14,0x12,0x16,0x15,0x12,0x13,0x14,0x16,0x14,0x53,0x16,0x14,0x12,0xf,0x14,0x13,0x14,0x16,0x54,0x56,0x54,0x4f,0x4d,0x15,0x52,0x14,0x13,0x14,0x56,0x56,0x54,0x53,0x13,0x14,0x16,0x14,0x12,0xf,0xd,0x12,0xf,0x14,0x12,0x16,0x14,0x16,0x52,0x4f,0xd,0x12,0xf,0x14,0x12,0x16,0x15,0x12,0x13,0x14,0x16,0x14,0x53,0x16,0x14,0x12,0xf,0x14,0x13,0x14,0x16,0x54,0x56,0x56,0x56,0x1b,0x19,0x56,0x1b,0x19,0x16,0x14,0x12,0x16,0x11,0x12,0x11,0xf,0x56,0x56,0x1b,0x19,0x16,0x1b,0x11,0x12,0x14,0x16,0x1d,0x1e,0x1d,0x1b,0x56,0x1b,0x19,0x56,0x1b,0x19,0x16,0x16,0x14,0x12,0x16,0x1b,0x19,0x1b,0x56,0x16,0x17,0x16,0x1b,0x19,0x16,0x11,0x12,0x11,0xf,0x56,0x57,0x56,0x1b,0x19,0x56,0x1b,0x19,0x16,0x14,0x12,0x16,0x11,0x12,0x11,0xf,0x56,0x56,0x1b,0x19,0x16,0x1b,0x11,0x12,0x14,0x16,0x1d,0x1e,0x1d,0x1b,0x56,0x1b,0x19,0x56,0x1b,0x19,0x16,0x16,0x14,0x12,0x16,0x1b,0x19,0x1b,0x56,0x16,0x17,0x16,0x1b,0x19,0x16,0x11,0x12,0x11,0xf,0x56,0x54};
uint8_t nyanMainExtTreble[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

uint16_t trebleNoteCount = INTRO_TREBLE_NOTE_COUNT;

uint16_t noteIndexTreble = 0;
uint16_t noteExtIndexTreble = 0;
uint8_t noteExtBitPosTreble = 0;

uint8_t noteBeatCountTreble = 0;

//Will switch from intro to body
uint8_t * notesTreblePtr = nyanIntroTreble;
uint8_t * notesExtTreblePtr = nyanIntroExtTreble;


uint8_t nyanIntroBass[] = {0x0,0x0};
uint8_t nyanIntroExtBass[] = {0x3};

uint8_t nyanMainBass[] = {0x75,0x69,0x73,0x67,0x76,0x6a,0x71,0x65,0x78,0x6c,0x73,0x67,0x3a,0x2e,0x7a,0x78,0x76};
uint8_t nyanMainExtBass[] = {0x0,0x0,0x0};


uint16_t bassNoteCount = INTRO_BASS_NOTE_COUNT;

uint16_t noteIndexBass = 0;
uint16_t noteExtIndexBass = 0;
uint8_t noteExtBitPosBass = 0;

uint8_t noteBeatCountBass = 0;

//Will switch from intro to body
uint8_t * notesBassPtr = nyanIntroBass;
uint8_t * notesExtBassPtr = nyanIntroExtBass;


uint8_t isIntroTreble = 1;
uint8_t isIntroBass = 1;

uint8_t key_buttons = 0;

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








void nyan_timer_C4_init()
{
	//for 136 BMP and 1/16 note support, 64 prescl and 13787 clk
	TCC4.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_NORMAL_gc;
	TCC4.CTRLE = 0;
	TCC4.INTCTRLA = TC_OVFINTLVL_HI_gc;
	TCC4.CTRLA = 0;
	TCC4.PER = 13787 ;
	
}

//Treble Counter
void note_0_timer_C5_init()
{

	TCC5.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_FRQ_gc;
	TCC5.CTRLE = TC_CCAMODE_COMP_gc; // | TC_CCBMODE_COMP_gc;
	TCC5.CTRLA = TC_CLKSEL_DIV8_gc;
}

//Base Counter
void note_1_timer_D5_init()
{
	
	TCD5.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_FRQ_gc;
	TCD5.CTRLE = TC_CCAMODE_COMP_gc; // | TC_CCBMODE_COMP_gc;
	TCD5.CTRLA = TC_CLKSEL_DIV8_gc;
}

void sys_timer_D5_init()
{
	TCD5.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_NORMAL_gc;
	TCD5.CTRLE = 0;
	TCD5.INTCTRLB = TC_OVFINTLVL_HI_gc;
	TCD5.CTRLA = TC_CLKSEL_DIV1024_gc;	
	TCD5.PER = 312; //100Hz
}

//Returns the clock count of the note, sets the beat counter for the note, and updates the indexes
uint16_t setNoteAndBeat(uint8_t * noteBeatCountPtr, uint8_t * notesPtr, uint8_t * noteExtPtr, uint16_t * noteIndexPtr, uint16_t * noteExtIndexPtr, uint8_t * extBitPosPtr)
{
	*noteBeatCountPtr = 0x01;
	uint8_t noteVal = notesPtr[*noteIndexPtr];
	uint8_t beats = *noteBeatCountPtr;
	beats <<= (noteVal >> 6);
	beats -= 1; //This will turn 0b0010 into 0b0001 or 0b0100 into 0b0011. A cheap way of doing a power of 2
	
	//0b00000001 << 3 = 0b00001000. 0b00001000 - 1 = 0b00000111
	//when we count down the number of 1/16 beats the note will be played for it will always play 1 1/16,
	//then decrement the beat counter until it reaches 0. So, a beat of 7 will play 8 beats, or a half note
	uint8_t extByte = noteExtPtr[*noteExtIndexPtr];
	if ((extByte >> (*extBitPosPtr)) & 0x01) //if the extension bit is set then this is a whole note
	{
		beats = 0x0F;
	}
	*noteBeatCountPtr = beats;
	//Get the counts from the note as an index
	uint16_t index = noteVal & 0x3F;
	uint16_t count = 0;
	if (index > 0)
	{
		count = noteClocks[index] / 2;
	}
	
	
	//Increment the note index and the bit pos of the ext
	(*noteIndexPtr)++;
	(*extBitPosPtr)++;
	
	//If we have maxed out the bit pos, move to the next ext byte
	if (*extBitPosPtr == 8)
	{
		*extBitPosPtr = 0;
		(*noteExtIndexPtr)++;
	}
	
	return count;
	
}


//Uses the current note indexes for treble
void setNoteAndBeatTreble()
{
	//The note number is the first 6 bits of the byte for that note and represents the index into the note clocks
	TCC5.CCA = setNoteAndBeat(&noteBeatCountTreble, notesTreblePtr, notesExtTreblePtr, &noteIndexTreble, &noteExtIndexTreble, &noteExtBitPosTreble);
}

//Uses the current note indexes for Bass
void setNoteAndBeatBass()
{
	//The note number is the first 6 bits of the byte for that note and represents the index into the note clocks
	TCD5.CCA = setNoteAndBeat(&noteBeatCountBass, notesBassPtr, notesExtBassPtr, &noteIndexBass, &noteExtIndexBass, &noteExtBitPosBass);
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


uint8_t expander_read_port(uint8_t portAddr)
{
	uint8_t port_val;
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


void nyan_init()
{
	notesTreblePtr = nyanIntroTreble;
	notesExtTreblePtr = nyanIntroExtTreble;
	noteBeatCountTreble = 0;
	noteIndexTreble = 0;
	noteExtIndexTreble = 0;
	noteExtBitPosTreble = 0;
	
	notesBassPtr = nyanIntroBass;
	notesExtBassPtr = nyanIntroExtBass;
	noteBeatCountBass = 0;
	noteIndexBass = 0;
	noteExtIndexBass = 0;
	noteExtBitPosBass = 0;
	
	setNoteAndBeatTreble();
	setNoteAndBeatBass();
	
	
}

void note_decay_init()
{
		////////////////////////////////////////// Timer and Event config ?
	EVSYS.CH1MUX =
		EVSYS_CHMUX_TCC4_OVF_gc;        // Event ch1 = tcc4 overflow
	TCC4.PER = 1200;
	TCC4.CTRLA = TC_CLKSEL_DIV1024_gc;
		
		////////////////////////////////////////// DAC config
	DACA.CTRLB =
		DAC_CHSEL_SINGLE_gc |          // DAC ch0 is active
		DAC_CH0TRIG_bm;			// DAC ch0 auto triggered by an event (CH1)
	DACA.CTRLC =
		DAC_REFSEL_AVCC_gc;             // Use AVCC (3.3v), right adj
	DACA.EVCTRL =
		DAC_EVSEL_1_gc;                 // Event Ch1 triggers the DAC conversion
		DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;  // enable DACA channel 0		
		
	EDMA.CH0.CTRLA = EDMA_CH_RESET_bm;
	EDMA.CH0.ADDRCTRL =
		EDMA_CH_RELOAD_TRANSACTION_gc |   // Reload after transaction
		EDMA_CH_DIR_INC_gc;		  // increment source address
	EDMA.CH0.TRIGSRC =
		EDMA_CH_TRIGSRC_DACA_CH0_gc;	  // DACA Ch0 is trigger source
	
	EDMA.CH0.CTRLB =
		EDMA_CH_TRNIF_bm |               //  Clear flag
		EDMA_CH_TRNINTLVL_HI_gc;          // For XMegaE5, this ISR is necessary to re-enable channel after transaction
	
	EDMA.CTRL = EDMA_ENABLE_bm; 	          // Enable, single buffer, round robin
}

void run_note_decay(uint16_t *data, uint8_t len)
// data[] contains a lookup table of length (len)
{
	
	EDMA.CH0.TRFCNT = len*2;                  // data array has len values
	EDMA.CH0.ADDR = (uint16_t)data;           // this is the source SRAM address
	EDMA.CH0.CTRLA =
		EDMA_CH_ENABLE_bm |               //   enable EDMA Ch0
		EDMA_CH_SINGLE_bm |               //   one burst per trigger
		EDMA_CH_BURSTLEN_bm;              //   2 bytes per burst
}

void note_play()
{
	

	PORTA.OUTSET = PIN3_bm;
	TCC5.CCA = noteClocks[STARTING_NOTE_INDEX + key_buttons];
	run_note_decay(exp_decay_vals, 13);
	TCC5.CTRLA = TC_CLKSEL_DIV8_gc;// TC_CLKSEL_DIV1024_gc;
}

void reset_play()
{
	PORTA.OUTCLR = PIN3_bm;
	EDMA.CH0.CTRLB |= EDMA_CH_TRNIF_bm;   // clear INT flag    // EDMA.INTFLAGS = EDMA_CH0TRNFIF_bm;    // alternate flag location also works
	TCC5.CNT = 0;
	TCC5.CTRLA = 0;// TC_CLKSEL_DIV1024_gc;
}

void update_mode_leds(uint8_t mode)
{
	uint8_t temp = PORTD.OUT & 0x1F;
	temp |= ((mode + 1) << 5);
	PORTD.OUT = temp;
}


void start_up(uint8_t mode)
{
	PORTD.DIRSET = PIN5_bm | PIN6_bm | PIN7_bm;
	PORTD.OUTCLR = PIN5_bm | PIN6_bm | PIN7_bm;	
	PORTA.DIRSET = PIN3_bm;
	PORTA.DIRSET = PIN2_bm;
	PORTC.DIRSET = PIN7_bm;
	
	update_mode_leds(mode);
	
	switch (mode)
	{
		case 0: //Free Play
			note_0_timer_C5_init();
			sys_timer_D5_init();
			note_decay_init();
			PORTC.OUTSET = PIN7_bm; //Turn on hall sensors 
			PORTC.DIRSET = PIN4_bm;
			PORTA.DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
			PORTA.PIN4CTRL = PORT_OPC_PULLUP_gc;
			PORTA.PIN5CTRL = PORT_OPC_PULLUP_gc;
			PORTA.PIN6CTRL = PORT_OPC_PULLUP_gc;
			PORTA.PIN7CTRL = PORT_OPC_PULLUP_gc;
			break;
		case 1: //Nyan
			nyan_timer_C4_init();
			note_0_timer_C5_init();			
			note_1_timer_D5_init();			
			nyan_init();
			
			
			PORTC.OUTCLR = PIN7_bm; //Turn off hall sensors 
			PORTD.DIRSET = PIN4_bm;
			PORTC.DIRSET = PIN4_bm; 	
			PORTA.OUTSET = PIN2_bm; //Turn on speaker channel
			//Start NYAN
			TCC4.CTRLA = TC_CLKSEL_DIV256_gc;
			break;
		case 2:
		case 3:
		case 4:
		default:
			break;
		
	}
	
}

uint8_t offset = 0;
void test_load_leds()
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

void eeprom_init()
{

	PORTA.DIRCLR = PIN0_bm;
	PORTA.PIN0CTRL = PORT_OPC_PULLUP_gc;
	start_mode = get_mode_from_eeprom();

}


uint8_t display_ctr = 0;

int main(void)
{

	uint8_t played = 0;
	uint16_t last_pos = 0;
	
	cli();
	clk_init();
	interrupt_init();
	twi_init();

	sei();

	
	

	oled_init();
 	expander_init();
	eeprom_init();

	start_up(start_mode);
	ws2812drv_init(); //Right now, this has to come after note decay edma setup
	uint16_t exp_val = 0;
	PORTA.OUTSET = PIN3_bm;
	
	while(!(PORTA.IN & PIN0_bm));
	ssd1306_setCursor(1, 0);
	ssd1306__printf("Mode: %d.", (start_mode + 1));
	
	char bits[17] = {'0'};
	bits[16] = '\0';
	while (1)
	{
		if (!(PORTA.IN & PIN0_bm))
		{
			_delay_ms(100);
			start_mode++;
			
			if (start_mode > MAX_MODE)
			{
				start_mode = 0;
			}
			
			write_mode_to_eeprom(start_mode);
			while(!(PORTA.IN & PIN0_bm));
			
			enable_wdt();
		}
				

		exp_val = expander_read_ports();

		

	
		if (start_mode == 0)
		{
			key_buttons = ((~PORTA.IN) & 0xF0) >> 4;
			ssd1306_setCursor(3, 0);
			ssd1306__printf("buttons: %d", key_buttons);
			
			if (key_buttons && exp_val != last_pos && !(played))
			{

				played = 1;
				note_play();
			}
			
			if (exp_val == last_pos)
			{
				played = 0;
			}
			
			if (exp_val != 0xffff)
			{
				d_to_b(exp_val, bits);
				last_pos = exp_val;
			}
			
			ssd1306_setCursor(2,0);
			ssd1306_displayString(bits);			
		
			if (display_ctr >= 10)	
			{
				test_load_leds();
				ws2812drv_start_transfer(leds, 50);
				display_ctr = 0;
				
			}
			
		}
		
	
 		//_delay_ms(500);
		PORTA.OUTTGL = PIN3_bm;
	}
}

ISR(EDMA_CH0_vect)
{
	reset_play();
	//EDMA.CH0.CTRLA |= EDMA_CH_ENABLE_bm;   // re-enable EDMA Ch0 after trans complete
}


ISR(TCD5_CCA_vect)
{
	display_ctr++;
}

ISR(TCC4_OVF_vect)
{
	PORTA.OUTSET = PIN2_bm;
	TCC4.INTFLAGS = TC4_OVFIF_bm;

	if (!(noteBeatCountTreble))
	{

		
		//First turn off treble counter
		TCC5.CTRLA = 0;
		TCC5.CCA = 0;

		if (noteIndexTreble >= trebleNoteCount)
		{
			if (isIntroTreble)
			{
				isIntroTreble = 0;
				trebleNoteCount = MAIN_TREBLE_NOTE_COUNT;
				notesTreblePtr = nyanMainTreble;
				notesExtTreblePtr = nyanMainExtTreble;
			}
			
			noteBeatCountTreble = 0;
			noteIndexTreble = 0;
			noteExtIndexTreble = 0;
			noteExtBitPosTreble = 0;
		}
		

		setNoteAndBeatTreble();
		
	}
	else
	{
		noteBeatCountTreble--;
	}
	
	if (!(noteBeatCountBass))
	{
		//First turn off base counter
		
		TCD5.CTRLA = 0;
		TCD5.CCA = 0;

		if (noteIndexBass >= bassNoteCount)
		{
			if (isIntroBass)
			{
				isIntroBass = 0;
				bassNoteCount = MAIN_BASS_NOTE_COUNT;
				notesBassPtr = nyanMainBass;
				notesExtBassPtr = nyanMainExtBass;
			}
			
			noteBeatCountBass = 0;
			noteIndexBass = 0;
			noteExtIndexBass = 0;
			noteExtBitPosBass = 0;
		}

		setNoteAndBeatBass();
	}
	else
	{
		noteBeatCountBass--;
	}
	
	
	//TODO: Update display and start transfer
					test_load_leds();
					ws2812drv_start_transfer(leds, 50);
	_delay_ms(20);
	TCC5.CTRLA = TC_CLKSEL_DIV8_gc;
	TCD5.CTRLA = TC_CLKSEL_DIV8_gc;
}