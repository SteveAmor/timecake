
// pine interrupt handler code

#include <nrf.h>
#include "nrf_gpio.h"
#include "clock.h"

#include "irq_pine.h"

#define BUTTON_PIN_IN  13

int button_state=0;
int touch_state=0;

// this reads and clears DOWN/UP flags
int button_read(void)
{
	int b=button_state;
	button_state&=0x1; // clear state change flags ( very small chance to miss a button press here :( )
	return b;
}

int touch_screen(void)
{
	int b=touch_state;
	touch_state = 0; // clear state change flag ( very small chance to miss a touch press here :( )
	return b;
}

static long long int debounce=0;

void GPIOTE_IRQHandler(void)
{
	if(NRF_GPIOTE->EVENTS_IN[0])
	{
		NRF_GPIOTE->EVENTS_IN[0] = 0;
		touch_state=1;
	}

	if(NRF_GPIOTE->EVENTS_IN[1]) // toggle
	{
		NRF_GPIOTE->EVENTS_IN[1] = 0;

		// this might be unsafe?
		int b=nrf_gpio_pin_read(BUTTON_PIN_IN);
		
		int nobounce=0;
		long long int d=clock_time();
		if( (d-debounce) > 0x0400) { nobounce=1; } // require more than 1/64 of a sec between state changes

		if(b)
		{
			
			if( nobounce && ((button_state&1)==0) ) { button_state|=2; }
			button_state|=1;
		}
		else
		{
			if( nobounce && ((button_state&1)==1) ) { button_state|=4; }
			button_state&=6;
		}
		
		if(nobounce)
		{
			debounce=d; // remember last time
		}


	}


}