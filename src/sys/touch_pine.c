
// pine touch code

#include <nrf.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "i2c_pine.h"

#define TOUCH_I2C_DEVICE 0x15
#define TOUCH_PIN_RESET 10
#define TOUCH_PIN_INTERRUPT 28


static unsigned char touch_data[128];


int touch_setup(void)
{

	nrf_gpio_cfg_output(TOUCH_PIN_RESET);

	// hardware reset
	nrf_gpio_pin_write(TOUCH_PIN_RESET,0);
	nrf_delay_ms(20);
	nrf_gpio_pin_write(TOUCH_PIN_RESET,1);
	nrf_delay_ms(400);

    nrf_gpio_cfg_sense_input(TOUCH_PIN_INTERRUPT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);

	NVIC_EnableIRQ(GPIOTE_IRQn);
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Msk;
	NRF_GPIOTE->CONFIG[0] =
		(GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos)	|
		(TOUCH_PIN_INTERRUPT << GPIOTE_CONFIG_PSEL_Pos)					|
		(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)			;

	return 0;
} 

unsigned char * touch_read(void)
{
/*
	acc_data[0]=0x2D;
	acc_data[1]=0x08;
	i2c_write(ACC_I2C_DEVICE,1,acc_data);
*/
	
	i2c_read(TOUCH_I2C_DEVICE,0,128,touch_data);
	
	return touch_data;
}

