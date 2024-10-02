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
#if 0
	oled.Refresh();
	for (int i = 0; i < 3; i++) {
		oled.Flash(true);
		::sleep_ms(500);
		oled.Flash(false);
		::sleep_ms(500);
	}
#endif
#if 0
	for (int x = 0; x < 128; x++) {
		oled.Clear();
		for (int i = 0; i < 32; i++) {
			oled.DrawHLine(x, i * 2, i);
		}
		oled.Refresh();
		::sleep_ms(100);
	}
#endif
	for (int y = 0; y < 64; y++) {
		oled.Clear();
		for (int i = 0; i < 64; i++) {
			oled.DrawVLine(i, y, i);
		}
		oled.Refresh();
		::sleep_ms(100);
	} while (0);
	for (;;) ;
}
