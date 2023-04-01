/*
 * xmega-cap-touch-ic-test.c
 *
 * Created: 2023-03-01 19:35:41
 * Author : Branden
 */ 

#define F_CPU 32000000U
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TWI_BAUD 148
#define CAP_SENSE_ADDR 0x50
#define CAP_SENSE_PRODUCT_ID_REG 0xFD
#define CAP_SENSE_VENDOR_ID_REG 0xFE
#define CAP_SENSE_REVISION_REG 0xFD
#define CAP_SENSE_MAIN_CTRL 0x00
#define CAP_SENSE_STATUS_1_REG 0x02
#define CAP_SENSE_STATUS_2_REG 0x03

volatile uint8_t twi_no_of_bytes,twi_data_buffer[10],twi_data_count,twi_transfer_complete, status, buttons;
#define TWI_WRITE 0x00
#define TWI_READ 0x01

#define CLK_SCALE 1024

uint16_t exp_decay_vals[] = {4095, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 0};
uint16_t note_freqs[] = {233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554};
uint16_t note_periods[16];

void init_clk()
{
	CCP = CCP_IOREG_gc;
	OSC_CTRL = OSC_RC32MEN_bm;
	
	while(!(OSC_STATUS & OSC_RC32MRDY_bm)){};
	
	CCP = CCP_IOREG_gc;
	CLK_CTRL = CLK_SCLKSEL_RC32M_gc;
	
	
}

void note_per_init()
{
	uint16_t scale = F_CPU/CLK_SCALE/2;
	
	for (uint8_t i = 0; i < 16; i++)
	{
		note_periods[i] = scale/note_freqs[i];
	}
		
}

void init_interrupts()
{
	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
}

void init_gpios()
{
	
}


void init_twi()
{
	TWIC_MASTER_BAUD=TWI_BAUD; //baud rate is set such that ftwi=100KHz
	TWIC_CTRL=0x00; //SDA hold time off, normal TWI operation
	TWIC_MASTER_CTRLA|=TWI_MASTER_INTLVL_gm|TWI_MASTER_RIEN_bm|TWI_MASTER_WIEN_bm|TWI_MASTER_ENABLE_bm; //enable high priority read and write interrupt, enable MASTER
	TWIC_MASTER_CTRLB=0x00; //no inactive bus timeout, no quick command and smart mode enabled
	TWIC_MASTER_CTRLC=0x00; //initially send ACK and no CMD selected
	TWIC_MASTER_STATUS|=TWI_MASTER_RIF_bm|TWI_MASTER_WIF_bm|TWI_MASTER_ARBLOST_bm|TWI_MASTER_BUSERR_bm|TWI_MASTER_BUSSTATE0_bm; //clear all flags initially and select bus state IDLE
}



volatile uint8_t prod_id = 0;
volatile uint8_t vend_id = 0;



void twi_read(uint8_t reg_addr, uint8_t len)
{
	twi_no_of_bytes=0;
	
	twi_data_count=0;
	
	twi_transfer_complete=0;
	
	twi_data_buffer[0]=reg_addr;
	
	TWIC_MASTER_ADDR=CAP_SENSE_ADDR|TWI_WRITE;
	
	while(twi_transfer_complete!=1);
	
	twi_no_of_bytes=len; twi_data_count=0;
	
	twi_transfer_complete=0;
	
	TWIC_MASTER_ADDR=CAP_SENSE_ADDR|TWI_READ;
	
	while(twi_transfer_complete!=1);
}


void twi_write(uint8_t reg_addr,uint8_t reg_data)
{
	twi_no_of_bytes=1;
	
	twi_data_count=0;
	
	twi_transfer_complete=0;
	
	twi_data_buffer[0]=reg_addr;
	
	twi_data_buffer[1]=reg_data;
	
	TWIC_MASTER_ADDR=CAP_SENSE_ADDR|TWI_WRITE;
	
	while(twi_transfer_complete!=1);
}

void init_timer_1()
{
	PORTC.DIRSET = PIN4_bm;
	//PORTC.REMAP = PORT_TC4A_bm;

	TCC5.CTRLB = TC_BYTEM_NORMAL_gc | TC_CIRCEN_DISABLE_gc | TC_WGMODE_FRQ_gc;
	TCC5.CTRLE = TC_CCAMODE_COMP_gc;


	
}



void test_get_prod_and_vend()
{
	twi_read(CAP_SENSE_PRODUCT_ID_REG, 1);
	prod_id = twi_data_buffer[0];
	vend_id = twi_data_buffer[1];
}

void multi_touch_init()
{
	twi_write(0x2A, 0b10001100); //multi touch
	twi_write(0x2B, 0); //multi touch pattern
	twi_write(0x1F, 0x3F); //sensitivity: lowest
	twi_write(0x20, 0b10100000); //SMBus config
	
	
}

void read_buttons()
{
	uint8_t ctrl = 0;
	twi_read(CAP_SENSE_MAIN_CTRL, 3);
	ctrl = 0;
	status = twi_data_buffer[2];
	buttons = twi_data_buffer[3];
	_delay_ms(2);
	twi_write(CAP_SENSE_MAIN_CTRL, ctrl & 0xFE); //clear INT


}

void test_edma_dac_event(uint16_t *data, uint8_t len)
// data[] contains a lookup table of length (len)
{
	PORTA.DIRSET = PIN2_bm;
	
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
	
	////////////////////////////////////////////  EDMA config
	EDMA.CTRL = EDMA_RESET_bm;
	EDMA.CH0.CTRLA = EDMA_CH_RESET_bm;
	EDMA.CH0.ADDRCTRL =
		EDMA_CH_RELOAD_TRANSACTION_gc |   // Reload after transaction
		EDMA_CH_DIR_INC_gc;		  // increment source address
	EDMA.CH0.TRIGSRC =
		EDMA_CH_TRIGSRC_DACA_CH0_gc;	  // DACA Ch0 is trigger source
	EDMA.CH0.TRFCNT = len*2;                  // data array has len values
	EDMA.CH0.ADDR = (uint16_t)data;           // this is the source SRAM address
	EDMA.CH0.CTRLA =
		EDMA_CH_ENABLE_bm |               //   enable EDMA Ch0
		EDMA_CH_SINGLE_bm |               //   one burst per trigger
		EDMA_CH_BURSTLEN_bm;              //   2 bytes per burst
	EDMA.CH0.CTRLB =
		EDMA_CH_TRNIF_bm |               //  Clear flag
		EDMA_CH_TRNINTLVL_HI_gc;          // For XMegaE5, this ISR is necessary to re-enable channel after transaction

	EDMA.CTRL = EDMA_ENABLE_bm; 	          // Enable, single buffer, round robin
	
	
	PMIC.CTRL = PMIC_HILVLEN_bm;		  //  enable interrupts
	DACA.CH0DATA = 4095;
}

void note_play()
{
	

	PORTA.OUTSET = PIN3_bm;
	TCC5.CCA = note_periods[buttons];// 31250;
	test_edma_dac_event(exp_decay_vals, 13);
	TCC5.CTRLA = TC_CLKSEL_DIV1024_gc;// TC_CLKSEL_DIV1024_gc;
}

uint16_t dac_data[] = {4095, 3071, 2047, 0};
uint8_t playing = 0;

void reset_play()
{
	PORTA.OUTCLR = PIN3_bm;
	EDMA.CH0.CTRLB |= EDMA_CH_TRNIF_bm;   // clear INT flag    // EDMA.INTFLAGS = EDMA_CH0TRNFIF_bm;    // alternate flag location also works
	TCC5.CNT = 0;
	TCC5.CTRLA = 0;// TC_CLKSEL_DIV1024_gc;	
}

int main(void)
{
	uint8_t last_buttons = 0;
	
	//PORTD.DIRSET = PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
	
	cli();
	note_per_init();
	init_clk();
	init_interrupts();
	init_gpios();
	init_twi();
	init_timer_1();
	//init_decay();
	
	sei();	
//	test_get_prod_id();
//	test_get_vend_id();
	PORTD.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	PORTD.OUTCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	_delay_ms(100);
	multi_touch_init();
//	test_edma_dac_event(exp_decay_vals, 13);
	PORTA.DIRCLR = PIN4_bm;
	PORTA.DIRSET = PIN3_bm;
	PORTA.PIN4CTRL = PORT_OPC_PULLUP_gc;

	uint8_t played = 0;
	
    while (1) 
    {
		read_buttons();
		
		PORTD.OUT = ((buttons >> 0) & 0x0F) << 0;
		if (!(PORTA.IN & PIN4_bm) && !(played) && buttons)
		{
			played = 1;

			
			note_play();
			
		}
		
		if (PORTA.IN & PIN4_bm)
		{
			played = 0;
			
		}
		
		/*
		if (last_buttons != buttons)
		{

			last_buttons = buttons;
			
			if (buttons)
			{
				
				note_play();
				
			}
			
		}
*/
    }
}


ISR(EDMA_CH0_vect)
{
	reset_play();
	//EDMA.CH0.CTRLA |= EDMA_CH_ENABLE_bm;   // re-enable EDMA Ch0 after trans complete
}

 ISR(TWIC_TWIM_vect)
 {
	 //If TWI arbitration is lost send STOP
	 
	 if(TWIC_MASTER_STATUS & (1<<TWI_MASTER_ARBLOST_bp))
	 {
		 TWIC_MASTER_CTRLC=(1<<TWI_MASTER_CMD1_bp)|(1<<TWI_MASTER_CMD0_bp); //send stop condition
	 }
	 
	 //If TWI bus error flag is set or NACK received then send STOP
	 
	 if((TWIC_MASTER_STATUS & (1<<TWI_MASTER_BUSERR_bp))||(TWIC_MASTER_STATUS & (1<<TWI_MASTER_RXACK_bp)))
	 {
		 TWIC_MASTER_CTRLC=TWI_MASTER_CMD_STOP_gc; //send stop condition
		 TWIC_MASTER_CTRLC=TWI_MASTER_BUSSTATE_IDLE_gc;
	 }

	if(twi_transfer_complete)
		return;

	 //If TWI write interrupt flag is set
	 
	 if(TWIC_MASTER_STATUS & (1<<TWI_MASTER_WIF_bp))
	 {
		 if(!(TWIC_MASTER_STATUS & (1<<TWI_MASTER_RXACK_bp)))      //check whether acknowledge is received or not
		 {
			 TWIC_MASTER_DATA=twi_data_buffer[twi_data_count++];
			 
			 if(twi_no_of_bytes)
			 {
				 twi_no_of_bytes--;
			 }
			 else
			 {
				 TWIC_MASTER_CTRLC=TWI_MASTER_CMD_STOP_gc; //send stop condition if all bytes are transferred
				 
				 twi_transfer_complete=1;
			 }
		 }
	 }
	 
	 //If RTC read interrupt flag is set
	 
	 if(TWIC_MASTER_STATUS & (1<<TWI_MASTER_RIF_bp))          //check whether read interrupt flag is set or not
	 {
		 twi_data_buffer[twi_data_count++]=TWIC_MASTER_DATA;
		 
		 if(twi_no_of_bytes==0)
		 {
			 TWIC_MASTER_CTRLC=(1<<TWI_MASTER_ACKACT_bp)|(1<<TWI_MASTER_CMD1_bp)|(1<<TWI_MASTER_CMD0_bp); //send NACK and stop condition if all bytes are transferred
			 
			 twi_transfer_complete=1;
		 }
		 else
		 {
			 twi_no_of_bytes--;
			 
			 TWIC_MASTER_CTRLC=(1<<TWI_MASTER_CMD1_bp)|(0<<TWI_MASTER_CMD0_bp); //send acknowledge
		 }
	 }
 }