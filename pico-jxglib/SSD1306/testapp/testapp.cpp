// To generate font files:
//   gurax ../../Font/script/GenerateFont.gura -o Font shinonome12,shinonome14,shinonome16 *.cpp
#include <stdio.h>
#include <jxglib/SSD1306.h>
#include "Font/shinonome12.h"
#include "Font/shinonome14.h"
#include "Font/shinonome16.h"
#include "jxglib/Font/shinonome18.h"

void SetupStage(SSD1306& oled, int iCase)
{
	oled.Clear((iCase == 1)? 0xff : 0x00);
	if (iCase == 2) oled.DrawRectFill(16, 24, 96, 16);
}

int main()
{
	::stdio_init_all();
	::i2c_init(i2c_default, 400000);
	::gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	::gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	::gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	::gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
	SSD1306 oled(i2c_default);
	oled.Initialize();
	//oled.Clear();
	//oled.Refresh();
	//::sleep_ms(1000);
	oled.SetFontScale(1, 1);
	oled.Clear();
	const char* str = "本日は晴天なりああ";
	oled.SetFont(Font::shinonome12);
	oled.DrawString(0, 0, str);
	oled.SetFont(Font::shinonome14);
	oled.DrawString(0, 16, str);
	oled.SetFont(Font::shinonome18);
	oled.DrawString(0, 32, str);
	oled.Refresh();
#if 0
#if 1
	for (int i = 0; i < 2; i++) {
		oled.Clear();
		oled.DrawString(0, 0, "本日は晴天なり");
		oled.DrawString(0, 32, "庭には二羽鶏がいる");
		oled.Refresh();
		::sleep_ms(1000);
		oled.Clear();
		oled.DrawString(0,  0, " !\"#$%&'()*+,-./");
		oled.DrawString(0, 32, "0123456789:;<=>?");
		oled.Refresh();
		::sleep_ms(1000);
		oled.Clear();
		oled.DrawString(0,  0, "@ABCDEFGHIJKLMNO");
		oled.DrawString(0, 32, "PQRSTUVWXYZ[\\]^_");
		oled.Refresh();
		::sleep_ms(1000);
		oled.Clear();
		oled.DrawString(0,  0, "`abcdefghijklmno");
		oled.DrawString(0, 32, "pqrstuvwxy{|}~");
		oled.Refresh();
		::sleep_ms(1000);
		oled.SetFontScale(1, 2);
	}
#endif
#if 1
	// SSD1306::Flash()
	for (int i = 0; i < 3; i++) {
		oled.Flash(true);
		::sleep_ms(500);
		oled.Flash(false);
		::sleep_ms(500);
	}
#endif
#if 1
	// SSD1306::DrawPixel()/ErasePixel()/InvertPixel()
	for (int iCase = 0; iCase < 3; iCase++) {
		SetupStage(oled, iCase);
		int x = 40, y = 16;
		int xDir = +1, yDir = +1;
		for (int i = 0; i < 1000; i++) {
			switch (iCase) {
			case 0: oled.DrawPixel(x, y); break; 
			case 1: oled.ErasePixel(x, y); break;
			case 2: oled.InvertPixel(x, y); break;
			default: break;
			}
			if (x + xDir < 0) xDir = +1;
			if (x + xDir >= oled.GetWidth()) xDir = -1;
			if (y + yDir < 0) yDir = +1;
			if (y + yDir >= oled.GetHeight()) yDir = -1;
			x += xDir, y += yDir;
			if (i % 10 == 0) oled.Refresh();
		}
		::sleep_ms(1000);
	} while (0);
#endif
#if 1
	// SSD1306::DrawHLine()/EraseHLine()/InvertHLine()
	for (int iCase = 0; iCase < 3; iCase++) {
		for (int x = -64; x < oled.GetWidth(); x++) {
			SetupStage(oled, iCase);
			for (int i = 0; i < oled.GetHeight(); i++) {
				switch (iCase) {
				case 0: oled.DrawHLine(x, i, i); break;
				case 1: oled.EraseHLine(x, i, i); break;
				case 2: oled.InvertHLine(x, i, i); break;
				default: break;
				}
				
			}
			oled.Refresh();
		}
		::sleep_ms(1000);
	}
#endif
#if 1
	// SSD1306::DrawVLine()/EraseVLine()/InvertVLine()
	for (int iCase = 0; iCase < 3; iCase++) {
		for (int y = -64; y < oled.GetHeight(); y++) {
			SetupStage(oled, iCase);
			for (int i = 0; i < oled.GetHeight(); i++) {
				switch (iCase) {
				case 0:
					oled.DrawVLine(i, y, i);
					oled.DrawVLine(i + oled.GetHeight() * 1, oled.GetHeight() - y - 1, -i);
					break;
				case 1:
					oled.EraseVLine(i, y, i);
					oled.EraseVLine(i + oled.GetHeight() * 1, oled.GetHeight() - y - 1, -i);
					break;
				case 2:
					oled.InvertVLine(i, y, i);
					oled.InvertVLine(i + oled.GetHeight() * 1, oled.GetHeight() - y - 1, -i);
					break;
				default:
					break;
				}
			}
			oled.Refresh();
		}
		::sleep_ms(1000);
	}
#endif
#if 0
	// SSD1306::DrawRectFill()
	for (int i = 0; i < oled.GetHeight(); i++) {
		oled.Clear();
		oled.DrawRectFill(0, 0, i * 2, i);
		oled.Refresh();
		::sleep_ms(100);
	}
	for (int i = 0; i < oled.GetHeight(); i++) {
		oled.Clear();
		oled.DrawRectFill(oled.GetWidth() - 1, oled.GetHeight() - 1, -i * 2, -i);
		oled.Refresh();
		::sleep_ms(100);
	}
#endif
#endif
	for (;;) ;
}
