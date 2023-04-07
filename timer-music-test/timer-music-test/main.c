/*
 * timer-music-test.c
 *
 * Created: 2023-03-31 19:49:49
 * Author : Branden
 */ 

#define F_CPU 32000000U
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define INTRO_TREBLE_NOTE_COUNT 31
#define MAIN_TREBLE_NOTE_COUNT 212

#define INTRO_BASS_NOTE_COUNT 2
#define MAIN_BASS_NOTE_COUNT 17

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

void clk_init()
{
	CCP = CCP_IOREG_gc;
	OSC_CTRL = OSC_RC32MEN_bm;
	
	while(!(OSC_STATUS & OSC_RC32MRDY_bm)){};
	
	CCP = CCP_IOREG_gc;
	CLK_CTRL = CLK_SCLKSEL_RC32M_gc;
	
	
}

void interrupts_init()
{
	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
}


void timer_C4_init()
{
	//for 136 BMP and 1/16 note support, 64 prescl and 13787 clk
	TCC4.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_NORMAL_gc;
	TCC4.CTRLE = 0;
	TCC4.INTCTRLA = TC_OVFINTLVL_HI_gc;
	TCC4.CTRLA = 0;	
	TCC4.PER = 13787 ;
	
}

//Treble Counter
void timer_C5_init()
{
	PORTC.DIRSET = PIN4_bm;
	//PORTC.REMAP = PORT_TC4A_bm;

	TCC5.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_FRQ_gc;
	TCC5.CTRLE = TC_CCAMODE_COMP_gc | TC_CCBMODE_COMP_gc;
	TCC5.CTRLA = TC_CLKSEL_DIV8_gc;
}

//Base Counter
void timer_D5_init()
{
	PORTD.DIRSET = PIN4_bm;
	//PORTC.REMAP = PORT_TC4A_bm;
	
	TCD5.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_FRQ_gc;
	TCD5.CTRLE = TC_CCAMODE_COMP_gc | TC_CCBMODE_COMP_gc;
	TCD5.CTRLA = TC_CLKSEL_DIV8_gc;
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

int main(void)
{
    /* Replace with your application code */
	cli();
	clk_init();
	interrupts_init();
	timer_C4_init();
	timer_C5_init();
	timer_D5_init();
	sei();
	
	PORTA.DIRSET = PIN3_bm;
	
	
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
	
	TCC4.CTRLA = TC_CLKSEL_DIV256_gc;
	
	
	
	//TCC5.CCA = 50;
    while (1) 
    {
		//
		//
		//_delay_ms(1000);
		//PORTA.OUTTGL = PIN3_bm;
		//TCD5.CCA = 170;
		//_delay_ms(1000);
		//PORTA.OUTTGL = PIN3_bm;
		//TCD5.CCA = 0;
    }
}

ISR(TCC4_OVF_vect)
{
	PORTA.OUTTGL = PIN3_bm;
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
		_delay_ms(20);
	TCC5.CTRLA = TC_CLKSEL_DIV8_gc;
	TCD5.CTRLA = TC_CLKSEL_DIV8_gc;		
}