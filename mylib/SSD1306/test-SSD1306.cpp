#include <stdio.h>
#include "SSD1306.h"

int main()
{
	::stdio_init_all();
	::i2c_init(i2c_default, 400000);
	::gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	::gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	::gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	::gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
	SSD1306 oled;
	oled.Initialize();
	oled.Refresh();
	for (int i = 0; i < 3; i++) {
		oled.Flash(true);
		::sleep_ms(500);
		oled.Flash(false);
		::sleep_ms(500);
	}
	oled.DrawLine(0, 0, 100, 30, true);
	oled.Refresh();
	for (;;) ;
}
