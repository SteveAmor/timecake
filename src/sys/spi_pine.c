
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "lcd.h"

int spi_enable(void)
{
//	nrf_gpio_cfg_output(LCD_SCK);
//	nrf_gpio_cfg_output(LCD_MOSI);
//	nrf_gpio_cfg_input(LCD_MISO, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_pin_write(LCD_SELECT,0); // select LCD
	nrf_gpio_pin_write(LCD_COMMAND,1);
	nrf_gpio_pin_write(LCD_RESET,1);

//	NRF_SPI0->ENABLE=0;
/*
	NRF_SPI0->PSELSCK=LCD_SCK;
	NRF_SPI0->PSELMOSI=LCD_MOSI;
	NRF_SPI0->PSELMISO=LCD_MISO;
	NRF_SPI0->FREQUENCY=SPI_FREQUENCY_FREQUENCY_M8;

	NRF_SPI0->CONFIG=(0x03 << 1);
	NRF_SPI0->EVENTS_READY=0; */
	NRF_SPI0->ENABLE=(SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

	// hardware reset
	nrf_gpio_pin_write(LCD_RESET,0);
	nrf_delay_ms(10);
	nrf_gpio_pin_write(LCD_RESET,1);

	// software reset
	lcd_command(CMD_SWRESET); nrf_delay_ms(120);

	// sleep off
	lcd_command(CMD_SLPOUT);
			
	// select 444 pixel format (Which is fastest to write)
	lcd_color_mode(0x444);
	lcd_rotate(0);

	// set screen to black
//	int c=0x000000;
//	lcd_shader(0,0,240,240,lcd_shader_color,&c);

	lcd_command(CMD_INVON);
	lcd_command(CMD_NORON);

	lcd_backlight(0xff);

    return 0;
}

int spi_disable(void)
{
	NRF_SPI0->ENABLE = 0;
//    *(volatile uint32_t *)0x40003FFC = 0;
//    *(volatile uint32_t *)0x40003FFC;
//    *(volatile uint32_t *)0x40003FFC = 1;
    return 0;
}