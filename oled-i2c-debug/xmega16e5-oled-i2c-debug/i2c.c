#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c.h"

volatile uint8_t twi_transfer_complete, twi_repeat_mode, twi_repeat_cmd, twi_repeat_data, twi_repeat_cmd_sent;
volatile uint16_t twi_no_of_bytes, twi_data_count;
volatile uint8_t * twi_data;

//TODO: Add error handling

void twi_init()
{

	TWIC_MASTER_BAUD=TWI_BAUD; //baud rate is set such that ftwi=100KHz
	TWIC_CTRL=0x00; //SDA hold time off, normal TWI operation
	TWIC_MASTER_CTRLA|=TWI_MASTER_INTLVL_gm|TWI_MASTER_RIEN_bm|TWI_MASTER_WIEN_bm|TWI_MASTER_ENABLE_bm; //enable high priority read and write interrupt, enable MASTER
	TWIC_MASTER_CTRLB=0x00; //no inactive bus timeout, no quick command and smart mode enabled
	TWIC_MASTER_CTRLC=0x00; //initially send ACK and no CMD selected
	TWIC_MASTER_STATUS|=TWI_MASTER_RIF_bm|TWI_MASTER_WIF_bm|TWI_MASTER_ARBLOST_bm|TWI_MASTER_BUSERR_bm|TWI_MASTER_BUSSTATE0_bm; //clear all flags initially and select bus state IDLE
}


uint8_t twi_read(uint8_t device_addr, uint8_t * buffer, uint16_t len)
{
	
	twi_no_of_bytes=len;
	
	twi_data_count=0;
	
	twi_transfer_complete=0;
	

	TWIC_MASTER_ADDR=(device_addr << 1)|TWI_READ;
	
	while(twi_transfer_complete!=1);
	
	return 0;
}


uint8_t twi_repeat(uint8_t device_addr, uint8_t cmd, uint8_t data, uint16_t count)
{

	twi_repeat_mode = 1;

	twi_no_of_bytes=count;
	
	twi_data_count=0;
	
	twi_repeat_cmd_sent = 0;
	
	twi_transfer_complete=0;
	
	twi_repeat_cmd = cmd;
	
	twi_repeat_data=data;
	

	TWIC_MASTER_ADDR=(device_addr << 1)|TWI_WRITE;
	
	while(twi_transfer_complete!=1);

	return 0;
}

uint8_t twi_write(uint8_t device_addr, uint8_t * data, uint16_t len)
{
	
	twi_repeat_mode = 0;
	
	twi_no_of_bytes=len;
	
	twi_data_count=0;
	
	twi_transfer_complete=0;
	
	twi_data=data;
	

	TWIC_MASTER_ADDR=(device_addr << 1)|TWI_WRITE;
	
	while(twi_transfer_complete!=1);
	
	return 0;
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
	{
		TWIC_MASTER_STATUS |= TWI_MASTER_WIF_bm;
		return;
	}
	//If TWI write interrupt flag is set
	
	if(TWIC_MASTER_STATUS & (1<<TWI_MASTER_WIF_bp))
	{
		if(!(TWIC_MASTER_STATUS & (1<<TWI_MASTER_RXACK_bp)))      //check whether acknowledge is received or not
		{
			if (twi_repeat_mode)
			{
				if (twi_repeat_cmd_sent)
				{
					TWIC_MASTER_DATA=twi_repeat_data; //using the pointer as a normal variable	
					twi_data_count++;
				}
				{
					TWIC_MASTER_DATA=twi_repeat_cmd;
					twi_repeat_cmd_sent = 1;
				}
				
			}
			else
			{
				TWIC_MASTER_DATA=twi_data[twi_data_count++];	
			}
			
			
			
			if(twi_data_count == twi_no_of_bytes)
			{
				TWIC_MASTER_CTRLC=TWI_MASTER_CMD_STOP_gc; //send stop condition if all bytes are transferred
 				
				twi_transfer_complete=1;
			}
		}
	}
	
	//If RTC read interrupt flag is set
	
	if(TWIC_MASTER_STATUS & (1<<TWI_MASTER_RIF_bp))          //check whether read interrupt flag is set or not
	{
		twi_data[twi_data_count++]=TWIC_MASTER_DATA;
		
		if(twi_data_count == twi_no_of_bytes)
		{
			TWIC_MASTER_CTRLC=(1<<TWI_MASTER_ACKACT_bp)|(1<<TWI_MASTER_CMD1_bp)|(1<<TWI_MASTER_CMD0_bp); //send NACK and stop condition if all bytes are transferred
			
			twi_transfer_complete=1;
		}
		else
		{
			TWIC_MASTER_CTRLC=(1<<TWI_MASTER_CMD1_bp)|(0<<TWI_MASTER_CMD0_bp); //send acknowledge
		}
	}
}
