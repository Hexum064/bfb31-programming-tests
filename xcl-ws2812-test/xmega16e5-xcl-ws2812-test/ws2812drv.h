/*
 * ws2812drv.h
 *
 * Created: 22-01-2015 11:00:36
 *  Author: mikael.pedersen
 */ 


#ifndef WS2812DRV_H_
#define WS2812DRV_H_

#include <stdint.h>

#define XCL_DLYCONF_DISABLE_gc 0

typedef struct
{
    uint8_t g;
    uint8_t r;
    uint8_t b;
} ws2812drv_led_t;

extern void ws2812drv_init(void);
extern void ws2812drv_start_transfer(ws2812drv_led_t *p, uint16_t cnt);
extern uint8_t ws2812drv_busy(void);

#endif /* WS2812DRV_H_ */