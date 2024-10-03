#include <stdio.h>
#include "SSD1306.h"

void SSD1306::Initialize()
{
	raw.AllocBuff();
	raw.SetDisplayOnOff(0);				// set display off
	// memory mapping
	raw.SetMemoryAddressingMode(0);		// set memory address mode: horizontal addressng mode
	raw.SetDisplayStartLine(0);			// set display start line to 0
	raw.SetSegmentRemap(1);				// set segment re-map, column address 127 is mapped to SEG0
	raw.SetMultiplexRatio(DisplayHeight - 1);	// set multiplex ratio: Display height - 1
	raw.SetCOMOutputScanDirection(1);	// set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
	raw.SetDisplayOffset(0);			// set display offset: no offset
	raw.SetCOMPinsHardwareConfiguration(0, 0);
										// set COM (common) pins hardware configuration. Board specific magic number.
	raw.SetDisplayClockDivideRatioOscillatorFrequency(0, 8);
										// set display clock divide ratio: div ratio of 1, standard freq 
	raw.SetPrechargePeriod(1, 15);		// set pre-charge period: Vcc internally generated on our board
	raw.SetVcomhDeselectLevel(0x3);		// set VCOMH deselect level: 0.83 x Vcc
	raw.SetContrastControl(255);		// set contrast conrol
	raw.EntireDisplayOn(0);				// set entire display on to follow RAM content
	raw.SetNormalInverseDisplay(0);		// set normal (not inverted) display
	raw.ChargePumpSetting(1);			// set charge pump: Vcc internally generated on our board
	raw.DeactivateScroll();				// deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
	raw.SetDisplayOnOff(1);				// turn display on
}

void SSD1306::Refresh()
{
	raw.SetColumnAddress(0, BufferWidth - 1);
	raw.SetPageAddress(0, NumPages - 1);
	raw.WriteBuffer();
}

template<class Logic> void SSD1306::DrawHLineT(int x, int y, int width)
{
	if (!CheckCoord(y, DisplayHeight) || !AdjustCoord(&x, &width, DisplayWidth)) return;
	uint8_t data = 0b00000001 << (y & 0b111);
	uint8_t* p = raw.GetPointer(x, y);
	for (int i = 0; i < width; i++, p++) *p = Logic()(*p, data);
}

template<class Logic> void SSD1306::DrawVLineT(int x, int y, int height)
{
	if (!CheckCoord(x, DisplayWidth) || !AdjustCoord(&y, &height, DisplayHeight)) return;
	uint32_t bits = (0xffffffff << y) & ~(0xffffffff << (y + height));
	uint8_t* p = raw.GetPointer(x);
	for (int i = 0; i < NumPages; i++, p += BufferWidth, bits >>= 8) {
		*p = Logic()(*p, static_cast<uint8_t>(bits & 0b11111111));
	} 
}

template<class Logic> void SSD1306::DrawLineT(int x0, int y0, int x1, int y1)
{
	if (x0 == x1) {
		DrawVLineT<Logic>(x0, y0, y1 - y0);
	} else if (y0 == y1) {
		DrawHLineT<Logic>(x0, y0, x1 - x0);
	} else {
		int dx =  abs(x1-x0);
		int sx = x0<x1 ? 1 : -1;
		int dy = -abs(y1-y0);
		int sy = y0<y1 ? 1 : -1;
		int err = dx+dy;
		int e2;

		while (true) {
			DrawPixel(x0, y0);
			if (x0 == x1 && y0 == y1)
				break;
			e2 = 2*err;

			if (e2 >= dy) {
				err += dy;
				x0 += sx;
			}
			if (e2 <= dx) {
				err += dx;
				y0 += sy;
			}
		}
	}
}

void SSD1306::DrawHLine(int x, int y, int width)
{
	DrawHLineT<Logic_Draw>(x, y, width);
}

void SSD1306::DrawVLine(int x, int y, int width)
{
	DrawVLineT<Logic_Draw>(x, y, width);
}

void SSD1306::DrawLine(int x0, int y0, int x1, int y1)
{
	DrawLineT<Logic_Draw>(x0, y0, x1, y1);
}

void SSD1306::EraseHLine(int x, int y, int width)
{
	DrawHLineT<Logic_Erase>(x, y, width);
}

void SSD1306::EraseVLine(int x, int y, int height)
{
	DrawVLineT<Logic_Erase>(x, y, height);
}

void SSD1306::EraseLine(int x0, int y0, int x1, int y1)
{
	DrawLineT<Logic_Erase>(x0, y0, x1, y1);
}

void SSD1306::InvertHLine(int x, int y, int width)
{
	DrawHLineT<Logic_Invert>(x, y, width);
}

void SSD1306::InvertVLine(int x, int y, int height)
{
	DrawVLineT<Logic_Invert>(x, y, height);
}

void SSD1306::InvertLine(int x0, int y0, int x1, int y1)
{
	DrawLineT<Logic_Invert>(x0, y0, x1, y1);
}

void SSD1306::SortPair(int v1, int v2, int* pMin, int* pMax)
{
	if (v1 < v2) {
		*pMin = v1, *pMax = v2;
	} else {
		*pMin = v2, *pMax = v1;
	}
}

bool SSD1306::AdjustCoord(int* pV, int* pDist, int vLimit)
{
	int& v = *pV, &dist = *pDist;
	if (dist == 0) return false;
	if (dist < 0) {
		v += dist + 1, dist = -dist;
	}
	if (v < 0) {
		dist += v, v = 0;
	}
	if (v >= vLimit) return false;
	if (v + dist > vLimit) {
		dist = vLimit - v;
	}
	return true;
}
