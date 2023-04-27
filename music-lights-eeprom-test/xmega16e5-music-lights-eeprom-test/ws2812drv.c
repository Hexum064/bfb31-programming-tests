/*
 * ws2812drv.c
 *
 * Created: 22-01-2015 10:57:57
 *  Author: mikael.pedersen
 * Modified by: Branden Boucher to run on port D 
 */ 

#include "ws2812drv.h"
#include <avr/io.h>
// 
// #if F_CPU != 32000000UL
// #error "Reconfigure baudrate and BTC0 timing"
// #endif


void ws2812drv_init(void)
{                                                               
    // Setup EDMA channel 2(+1) 
    EDMA.CTRL = EDMA_ENABLE_bm | EDMA_CHMODE_STD02_gc | EDMA_DBUFMODE_DISABLE_gc | EDMA_PRIMODE_RR0123_gc;
    EDMA.CH2.CTRLB = 0;// EDMA_CH_INTLVL_OFF_gc;
    EDMA.CH2.ADDRCTRL = EDMA_CH_RELOAD_NONE_gc | EDMA_CH_DIR_INC_gc;
    EDMA.CH2.DESTADDRCTRL = EDMA_CH_RELOAD_NONE_gc | EDMA_CH_DESTDIR_FIXED_gc;
    EDMA.CH2.DESTADDR = (uint16_t)&USARTD0.DATA;
    EDMA.CH2.TRIGSRC = EDMA_CH_TRIGSRC_USARTD0_DRE_gc;
    // Setup port pins for TxD, XCK and LUT0OUT
    PORTD.PIN0CTRL = PORT_OPC_TOTEM_gc;                         // LUT0OUT (data to WS2812)
    PORTD.PIN1CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_RISING_gc;    // XCK
    PORTD.PIN3CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_LEVEL_gc;     // TxD
    PORTD.DIRSET = PIN0_bm | PIN1_bm | PIN3_bm;
	

    // Setup Event channel 0 to TxD (async)
    EVSYS.CH0MUX = EVSYS_CHMUX_PORTD_PIN3_gc;
    EVSYS.CH0CTRL = EVSYS_DIGFILT_1SAMPLE_gc;
    // Setup Event channel 6 to XCK rising edge
    EVSYS.CH6MUX = EVSYS_CHMUX_PORTD_PIN1_gc;
    EVSYS.CH6CTRL = EVSYS_DIGFILT_1SAMPLE_gc;

    // Setup USART in master SPI mode 1, MSB first
    USARTD0.BAUDCTRLA = 19;                                     // 800.000 baud (1250 ns @ 32 MHz)
    USARTD0.BAUDCTRLB = 0;
    USARTD0.CTRLA = 0;// USART_RXSINTLVL_OFF_gc | USART_RXCINTLVL_OFF_gc | USART_TXCINTLVL_OFF_gc;
    USARTD0.CTRLC = USART_CMODE_MSPI_gc | (1 << 1);             // UDORD=0 UCPHA=1
    USARTD0.CTRLD = USART_DECTYPE_DATA_gc | USART_LUTACT_OFF_gc | USART_PECACT_OFF_gc;
    USARTD0.CTRLB = USART_TXEN_bm;

    // Setup XCL BTC0 timer to 1shot pwm generation
    XCL.CTRLE = XCL_CMDSEL_NONE_gc | XCL_TCSEL_BTC0_gc | XCL_CLKSEL_DIV1_gc;
    XCL.CTRLF = XCL_CMDEN_DISABLE_gc | 0x03;                    // 0x03 : One-shot PWM (missing in iox32e5.h)
    XCL.CTRLG = XCL_EVACTEN_bm | (0x03<<3) | XCL_EVSRC_EVCH6_gc; // 0x03<<3 : EVACT0=RESTART (missing in iox32e5.h)
    XCL.PERCAPTL = 22;                                          // Output high time if data is 1 (from RESTART to falling edge of one-shot)
    XCL.CMPL = 13;                                              // Output high time if data is 0 (from RESTART to rising edge of one-shot)

    // Setup XCL LUT
    XCL.CTRLA = XCL_LUT0OUTEN_PIN0_gc | XCL_PORTSEL_PD_gc | XCL_LUTCONF_MUX_gc;  // Setup glue logic for MUX
    XCL.CTRLB = 0x50;                                           // IN3SEL=XCL, IN2SEL=XCL, IN1SEL=EVSYS, IN0SEL=EVSYS (missing in iox32e5.h)
    XCL.CTRLC = XCL_EVASYSEL0_bm | XCL_DLYCONF_DISABLE_gc;      // Async inputs, no delay
    XCL.CTRLD = 0xA0;                                           // LUT truthtables (only LUT1 is used)

}


void ws2812drv_start_transfer(ws2812drv_led_t *p, uint16_t cnt)
{
    EDMA.CH2.ADDR = (uint16_t)p;
    EDMA.CH2.TRFCNT = cnt * sizeof(ws2812drv_led_t);
    EDMA.CH2.CTRLA = EDMA_CH_ENABLE_bm | EDMA_CH_SINGLE_bm; // Start DMA transfer to LEDs
}


uint8_t ws2812drv_busy(void)
{
    return (EDMA.STATUS & (EDMA_CH2BUSY_bm | EDMA_CH2PEND_bm)) ? 1 : 0;
}
