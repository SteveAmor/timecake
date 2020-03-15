
// pine lcd code

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "lcd.h"
#include "spi_pine.h"


// send and receive data
int lcd_transfer(int t)
{
	volatile uint32_t r;
	NRF_SPI0->EVENTS_READY=0;           // ready
	NRF_SPI0->TXD=(uint32_t)t;          // out
    while(NRF_SPI0->EVENTS_READY==0){;} // wait
	r=NRF_SPI0->RXD;                    // in
	return (int)r;
}

// send a command byte
int lcd_command(int t)
{
	nrf_gpio_pin_write(LCD_COMMAND,0);
	return lcd_transfer(t);
}

// send a data byte
static inline int lcd_data(int t)
{
	nrf_gpio_pin_write(LCD_COMMAND,1);
	return lcd_transfer(t);
}

// send a data word
static inline void lcd_data_word(int t)
{
	nrf_gpio_pin_write(LCD_COMMAND,1);
	lcd_transfer(t>>8);
	lcd_transfer(t&0xff);
}

// select rectangular area to read from or write to
static inline void lcd_window(int px,int py,int hx,int hy)
{
	lcd_command(CMD_CASET);
	lcd_data_word(px);
	lcd_data_word((px+hx-1));

	lcd_command(CMD_RASET);
	lcd_data_word(py);
	lcd_data_word((py+hy-1));
}

/*

setup all LCD related stuff

*/
int lcd_setup(void)
{

	nrf_gpio_cfg_output(LCD_BACKLIGHT_LOW);
	nrf_gpio_cfg_output(LCD_BACKLIGHT_MID);
	nrf_gpio_cfg_output(LCD_BACKLIGHT_HIGH);	

	nrf_gpio_cfg_output(LCD_SCK);
	nrf_gpio_cfg_output(LCD_MOSI);
	nrf_gpio_cfg_input(LCD_MISO, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_cfg_output(LCD_SELECT);
	nrf_gpio_cfg_output(LCD_COMMAND);
	nrf_gpio_cfg_output(LCD_RESET);

	nrf_gpio_pin_write(LCD_SELECT,0); // select LCD
	nrf_gpio_pin_write(LCD_COMMAND,1);
	nrf_gpio_pin_write(LCD_RESET,1);

	NRF_SPI0->ENABLE=0;

	NRF_SPI0->PSELSCK=LCD_SCK;
	NRF_SPI0->PSELMOSI=LCD_MOSI;
	NRF_SPI0->PSELMISO=LCD_MISO;
	NRF_SPI0->FREQUENCY=SPI_FREQUENCY_FREQUENCY_M8;

	NRF_SPI0->CONFIG=(0x03 << 1);
	NRF_SPI0->EVENTS_READY=0;
	NRF_SPI0->ENABLE=(SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

	// hardware reset
	nrf_gpio_pin_write(LCD_RESET,0);
	nrf_delay_ms(200);
	nrf_gpio_pin_write(LCD_RESET,1);

	// software reset
	lcd_command(CMD_SWRESET); nrf_delay_ms(200);

	// sleep off
	lcd_command(CMD_SLPOUT);
			
	// select 444 pixel format (Which is fastest to write)
	lcd_color_mode(0x444);
	lcd_rotate(0);

	// set screen to black
	int c=0x000000;
	lcd_shader(0,0,240,240,lcd_shader_color,&c);

	lcd_command(CMD_INVON);
	lcd_command(CMD_NORON);

	lcd_backlight(0xff);
	
	
	return 0;
}


/*

Rotate the display address mode.

0 == normal
1 == rotate clockwise 90
2 == rotate clockwise 180
3 == rotate clockwise 270

*/
int lcd_rotate(int rot)
{
	switch(rot)
	{
		case 0:
			lcd_command(CMD_MADCTL); lcd_data(0x00);
			lcd_command(CMD_VSCSAD); lcd_data_word(0);
		break;
		case 1:
			lcd_command(CMD_MADCTL); lcd_data(0x60);
			lcd_command(CMD_VSCSAD); lcd_data_word(0);
		break;
		case 2:
			lcd_command(CMD_MADCTL); lcd_data(0xc0);
			lcd_command(CMD_VSCSAD); lcd_data_word(80);
		break;
		case 3:
			lcd_command(CMD_MADCTL); lcd_data(0xa0);
			lcd_command(CMD_VSCSAD); lcd_data_word(80);
		break;
		default:
			return 1; // error
		break;
	}
	return 0;
}

/*

set sleep mode.

0   awake
255 asleep

*/
int lcd_sleep(int sleep)
{
	if(sleep)
	{
		NRF_CLOCK->TASKS_HFCLKSTART = 0;
		while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 1);
		NRF_CLOCK->EVENTS_HFCLKSTARTED = 1;

		lcd_command(CMD_SLPIN);
		spi_disable();
		SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk);
		__DSB();
	}
	else
	{
		NRF_CLOCK->TASKS_HFCLKSTART = 1;
		while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
		NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;

		SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
		__DSB();
		spi_enable();
		lcd_command(CMD_SLPOUT);
	}
	return 0;
}

/*

Only 4 levels but use a bright value from 0 to 255

0x00 == off
0x40 == low
0x80 == medium
0xc0 == high

*/
int lcd_backlight(int bright)
{


	if(bright<0x40) // darkest
	{
		nrf_gpio_pin_write(LCD_BACKLIGHT_LOW,1);
		nrf_gpio_pin_write(LCD_BACKLIGHT_MID,1);
		nrf_gpio_pin_write(LCD_BACKLIGHT_HIGH,1);
	}
	else
	if(bright<0x80)
	{
		nrf_gpio_pin_write(LCD_BACKLIGHT_LOW,0);
		nrf_gpio_pin_write(LCD_BACKLIGHT_MID,1);
		nrf_gpio_pin_write(LCD_BACKLIGHT_HIGH,1);
	}
	else
	if(bright<0xc0)
	{
		nrf_gpio_pin_write(LCD_BACKLIGHT_LOW,1);
		nrf_gpio_pin_write(LCD_BACKLIGHT_MID,0);
		nrf_gpio_pin_write(LCD_BACKLIGHT_HIGH,1);
	}
	else
	{
		nrf_gpio_pin_write(LCD_BACKLIGHT_LOW,1);
		nrf_gpio_pin_write(LCD_BACKLIGHT_MID,1);
		nrf_gpio_pin_write(LCD_BACKLIGHT_HIGH,0);
	}

	// display on/off
	if(bright==0)
	{
		lcd_command(CMD_DISPOFF);
	}
	else
	{
		lcd_command(CMD_DISPON);
	}

	return 0;
}

int lcd_shader_color(int x,int y,void *data)
{
	int d=*((int*)data);
	return d;
}

// shade a rectangular area with a function
static void lcd_shader_565(int px,int py,int hx,int hy,int(*pixel)(int x,int y,void *data),void *data)
{
	volatile uint32_t r; // dummy read value

	nrf_gpio_pin_write(LCD_SELECT,0); // make sure we are talking to lcd
	lcd_window(px,py,hx,hy); // window 
	lcd_command(CMD_RAMWR); // write pixels
	nrf_gpio_pin_write(LCD_COMMAND,1); // data

	// state passed into lcd_shader_transfer by pointers
	int d1=0;

	int xymax=hx*hy; // need one more pixel loop than asked for
	for(int xy=0;xy<=xymax;xy++)
	{
		int x=px+(xy%hx);
		int y=py+(xy/hx);
		if(xy==0) // first transfer is read only
		{
			d1=(*pixel)(x,y,data);												// get first pixel at start
		}
		else // other transfers
		{
			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)(((d1>>16)&0xf8)|((d1>>13)&0x07));			// out
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;                    								// in
			r;//hush

			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)(((d1>>3)&0x1f)|((d1>>5)&0x70));			// out
			if(xy!=xymax) { d1=(*pixel)(x,y,data); }							// get pixel before waiting... (IMPORTANT)
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;													// in
			r;//hush
		}
	}
}

// shade a rectangular area with a function
static void lcd_shader_444(int px,int py,int hx,int hy,int(*pixel)(int x,int y,void *data),void *data)
{
	volatile uint32_t r; // dummy read value

	nrf_gpio_pin_write(LCD_SELECT,0); // make sure we are talking to lcd
	lcd_window(px,py,hx,hy); // window 
	lcd_command(CMD_RAMWR); // write pixels
	nrf_gpio_pin_write(LCD_COMMAND,1); // data

	// state passed into lcd_shader_transfer by pointers
	int d1=0;
	int d2=0;

	int xymax=hx*hy; // need one more pixel loop than asked for
	for(int xy=0;xy<=xymax;xy++)
	{
		int x=px+(xy%hx);
		int y=py+(xy/hx);
		if(xy&1) // odd
		{
			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)(((d1>>16)&0xf0)|((d1>>12)&0x0f));			// out
			if(xy!=xymax) { d2=(*pixel)(x,y,data); }							// get pixel before waiting... (IMPORTANT)
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;                    								// in
			r;//hush

			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)(((d1)&0xf0)|((d2>>20)&0x0f));				// out
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;													// in
			r;//hush
		}
		else // even
		{
			if(xy==0) // first
			{
				d1=(*pixel)(x,y,data);												// get first pixel at start
			}
			else
			{
				NRF_SPI0->EVENTS_READY=0;											// ready
				NRF_SPI0->TXD=(uint32_t)(((d2>>8)&0xf0)|((d2>>4)&0x0f));			// out
				if(xy!=xymax) { d1=(*pixel)(x,y,data); }							// get pixel before waiting... (IMPORTANT)
				while(NRF_SPI0->EVENTS_READY==0){;}									// wait
				r=NRF_SPI0->RXD;													// in
				r;//hush
			}
		}
	}
}

// shade a rectangular area with a function
static void lcd_shader_888(int px,int py,int hx,int hy,int(*pixel)(int x,int y,void *data),void *data)
{
	volatile uint32_t r; // dummy read value

	nrf_gpio_pin_write(LCD_SELECT,0); // make sure we are talking to lcd
	lcd_window(px,py,hx,hy); // window 
	lcd_command(CMD_RAMWR); // write pixels
	nrf_gpio_pin_write(LCD_COMMAND,1); // data

	// state passed into lcd_shader_transfer by pointers
	int d1=0;
	int xymax=hx*hy; // need one more pixel than asked for
	for(int xy=0;xy<=xymax;xy++)
	{
		int x=px+(xy%hx);
		int y=py+(xy/hx);
		if(xy==0) // first transfer is read only
		{
			d1=(*pixel)(x,y,data);												// get first pixel at start
		}
		else // other transfers
		{
			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)((d1>>16)&0xff);							// out
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;                    								// in
			r;//hush

			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)((d1>>8)&0xff);								// out
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;													// in
			r;//hush

			NRF_SPI0->EVENTS_READY=0;											// ready
			NRF_SPI0->TXD=(uint32_t)((d1)&0xff);								// out
			if(xy!=xymax) { d1=(*pixel)(x,y,data); }							// get pixel before waiting... (IMPORTANT)
			while(NRF_SPI0->EVENTS_READY==0){;}									// wait
			r=NRF_SPI0->RXD;													// in
			r;//hush
		}
	}
}

static int lcd_color_mode_value=0x444;

// shade a rectangular area with a function
void lcd_shader(int px,int py,int hx,int hy,int(*pixel)(int x,int y,void *data),void *data)
{
	switch(lcd_color_mode_value)
	{
		case 0x444:
			lcd_shader_444(px,py,hx,hy,pixel,data);
		break;
		case 0x565:
			lcd_shader_565(px,py,hx,hy,pixel,data);
		break;
		case 0x888:
			lcd_shader_888(px,py,hx,hy,pixel,data);
		break;
	}
}

// set/get color mode values are : 0x444 , 0x565 , 0x888 or 0x000 to just read.
int lcd_color_mode(int color_mode)
{
	nrf_gpio_pin_write(LCD_SELECT,0);
	switch(color_mode)
	{
		case 0x444:
			lcd_command(CMD_COLMOD); lcd_data(0x63);
			lcd_color_mode_value=0x444;
		break;
		case 0x565:
			lcd_command(CMD_COLMOD); lcd_data(0x65);
			lcd_color_mode_value=0x565;
		break;
		case 0x888:
			lcd_command(CMD_COLMOD); lcd_data(0x66);
			lcd_color_mode_value=0x888;
		break;
	}
	return lcd_color_mode_value;
}
