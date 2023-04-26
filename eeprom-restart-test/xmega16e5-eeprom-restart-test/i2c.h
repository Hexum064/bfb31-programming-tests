#ifndef I2C_H
#define I2C_H

#define TWI_BAUD 148
#define TWI_WRITE 0x00
#define TWI_READ 0x01

void twi_init();
uint8_t twi_read(uint8_t device_addr, uint8_t * buffer, uint16_t len);
uint8_t twi_write(uint8_t device_addr, uint8_t * data, uint16_t len);
uint8_t twi_repeat(uint8_t device_addr, uint8_t cmd, uint8_t data, uint16_t count);

#endif //I2C_H