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

template<class Logic> void SSD1306::DrawHLineT_NoAdjust(int x, int y, int width)
{
	uint8_t data = 0b00000001 << (y & 0b111);
	uint8_t* p = raw.GetPointer(x, y);
	for (int i = 0; i < width; i++, p++) *p = Logic()(*p, data);
}

template<class Logic> void SSD1306::DrawVLineT_NoAdjust(int x, int y, int height)
{
	uint32_t bits = (0xffffffff << y) & ~(0xffffffff << (y + height));
	int page;
	uint8_t* pTop = raw.GetPointer(x, y, &page);
	bits >>= page * 8;
	for (uint8_t* p = pTop; page < NumPages && bits; page++, p += BufferWidth, bits >>= 8) {
		*p = Logic()(*p, static_cast<uint8_t>(bits & 0b11111111));
	}
}

template<class Logic> void SSD1306::DrawHLineT(int x, int y, int width)
{
	if (!CheckCoord(y, DisplayHeight) || !AdjustCoord(&x, &width, DisplayWidth)) return;
	DrawHLineT_NoAdjust<Logic>(x, y, width);
}

template<class Logic> void SSD1306::DrawVLineT(int x, int y, int height)
{
	if (!CheckCoord(x, DisplayWidth) || !AdjustCoord(&y, &height, DisplayHeight)) return;
	DrawVLineT_NoAdjust<Logic>(x, y, height);
}

template<class Logic> void SSD1306::DrawLineT(int x0, int y0, int x1, int y1)
{
	if (x0 == x1) {
		DrawVLineT<Logic>(x0, y0, y1 - y0);
	} else if (y0 == y1) {
		DrawHLineT<Logic>(x0, y0, x1 - x0);
	} else {
		//int dx = abs(x1 - x0);
		//int sx = (x0 < x1)? 1 : -1;
		//int dy = -abs(y1 - y0);
		//int sy = (y0 < y1)? 1 : -1;
		int dx, dy, sx, sy;
		if (x0 < x1) {
			dx = x1 - x0;
			sx = +1;
		} else {
			dx = x0 - x1;
			sx = -1;
		}
		if (y0 < y1) {
			dy = y0 - y1;
			sy = +1;
		} else {
			dy = y1 - y0;
			sy = -1;
		}
		int err = dx + dy;
		int x = x0, y = y0;
		for (;;) {
			DrawPixelT<Logic>(x, y);
			if (x == x1 && y == y1) break;
			int err2 = 2 * err;
			if (err2 >= dy) {
				err += dy;
				x += sx;
			}
			if (err2 <= dx) {
				err += dx;
				y += sy;
			}
		}
	}
}

template<class Logic> void SSD1306::DrawRectT(int x, int y, int width, int height)
{
	int xLeft = x, xRight = x + width -1;
	int yTop = y, yBottom = y + height - 1;
	int xLeftAdjust = xLeft, widthAdjust = width;
	int yTopAdjust = yTop, heightAdjust = height;
	if (AdjustCoord(&xLeftAdjust, &widthAdjust, DisplayWidth)) {
		if (CheckCoord(yTop, DisplayHeight)) {
			DrawHLineT_NoAdjust<Logic>(xLeftAdjust, yTop, widthAdjust);
		}
		if (CheckCoord(yBottom, DisplayHeight)) {
			DrawHLineT_NoAdjust<Logic>(xLeftAdjust, yBottom, widthAdjust);
		}
	}
	if (AdjustCoord(&yTopAdjust, &heightAdjust, DisplayHeight)) {
		if (CheckCoord(xLeft, DisplayWidth)) {
			DrawVLineT_NoAdjust<Logic>(xLeft, yTopAdjust, heightAdjust);
		}
		if (CheckCoord(xRight, DisplayWidth)) {
			DrawVLineT_NoAdjust<Logic>(xRight, yTopAdjust, heightAdjust);
		}
	}
}

template<class Logic> void SSD1306::DrawRectFillT(int x, int y, int width, int height)
{
	if (!AdjustCoord(&x, &width, DisplayWidth) || !AdjustCoord(&y, &height, DisplayHeight)) return;
	uint32_t bits = (0xffffffff << y) & ~(0xffffffff << (y + height));
	int page;
	uint8_t* pTop = raw.GetPointer(x, y, &page);
	bits >>= page * 8;
	for (int i = 0; i < width; i++, pTop++) {
		for (uint8_t* p = pTop; page < NumPages && bits; page++, p += BufferWidth, bits >>= 8) {
			*p = Logic()(*p, static_cast<uint8_t>(bits & 0b11111111));
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

void SSD1306::DrawRect(int x, int y, int width, int height)
{
	DrawRectT<Logic_Draw>(x, y, width, height);
}

void SSD1306::DrawRectFill(int x, int y, int width, int height)
{
	DrawRectFillT<Logic_Draw>(x, y, width, height);
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

void SSD1306::EraseRect(int x, int y, int width, int height)
{
	DrawRectT<Logic_Erase>(x, y, width, height);
}

void SSD1306::EraseRectFill(int x, int y, int width, int height)
{
	DrawRectFillT<Logic_Erase>(x, y, width, height);
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

void SSD1306::InvertRect(int x, int y, int width, int height)
{
	DrawRectT<Logic_Invert>(x, y, width, height);
}

void SSD1306::InvertRectFill(int x, int y, int width, int height)
{
	DrawRectFillT<Logic_Invert>(x, y, width, height);
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
