
// pine touch code

#include <nrf.h>

#include "i2c_pine.h"

#define TOUCH_I2C_ADDRESS (0x15)

int touch_setup(void)
{
	i2c_setup(); // this will may called multiple times
	
	return 0;
}
