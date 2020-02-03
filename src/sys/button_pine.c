
// pine button code

#include <nrf.h>
#include "nrf_gpio.h"

#include "button.h"
#include "saveram.h"
#include "irq_pine.h"

#define BUTTON_PIN_IN  13
#define BUTTON_PIN_OUT 15


int button_setup(void)
{
	nrf_gpio_cfg_output(BUTTON_PIN_OUT);
    nrf_gpio_cfg_sense_input(BUTTON_PIN_IN, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
	nrf_gpio_pin_write(BUTTON_PIN_OUT,1);


	NVIC_EnableIRQ(GPIOTE_IRQn);
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN1_Msk;
	NRF_GPIOTE->CONFIG[1] =
		(GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos)	|
		(BUTTON_PIN_IN << GPIOTE_CONFIG_PSEL_Pos)						|
		(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)			;

	return 0;
}
