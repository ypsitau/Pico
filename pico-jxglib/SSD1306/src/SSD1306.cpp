//==============================================================================
// 128x64 Dot Matrix OLED/PLED Segment/Common Driver with Controller
// Specification: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
//==============================================================================
#include <stdio.h>
#include "jxglib/SSD1306.h"

//------------------------------------------------------------------------------
// SSD1306
//------------------------------------------------------------------------------
void SSD1306::Initialize()
{
	raw.AllocBuffer();
	raw.SetDisplayOnOff(0);							// Set Display ON/OFF = 0: OFF
	raw.SetMemoryAddressingMode(0);					// Set Memory Addressing Mode = 0: Horizontal Addressing Mode
	raw.SetDisplayStartLine(0);						// Set Display Start Line = 0
	raw.SetSegmentRemap(1);							// Set Segment Re-map = 1: column address 127 is mapped to SEG0
	raw.SetMultiplexRatio(GetHeight() - 1);	// Set Multiplex Ratio = GetHeight() - 1
	raw.SetCOMOutputScanDirection(1);				// Set COM Output Scan Direction = 1: remapped mode. Scan from COM[N-1] to COM0
	raw.SetDisplayOffset(0);						// Set Display Offset = 0
	raw.SetCOMPinsHardwareConfiguration((GetHeight() == 64)? 1 : 0, 0);
													// Set COM Pins Hardware Configuration
													//   A[4] = 0: Sequential COM pin configuration
													//          1: Alternative COM pin configuration
													//   A[5] = 0: Disable COM Left/Right remap
	raw.SetDisplayClockDivideRatioOscillatorFrequency(0, 8);
													// Set Display Clock Divide Ratio/Oscillator Frequency
													//   A[3:0] = 0: divide ratio is 1
													//   A[7:4] = 8: oscilator frequency
	raw.SetPrechargePeriod(1, 15);					// Set Pre-charge Period
													//   A[3:0] = 1: Phase 1 period to 1
													//   A[7:4] = 15: Phase 2 period to 15
	raw.SetVcomhDeselectLevel(0x3);					// Set Vcomh Deselect Level = 3: up to 0.83 x Vcc
	raw.SetContrastControl(255);					// Set Contrast Conrol = 255
	raw.EntireDisplayOn(0);							// Entire Display ON = 0: Resume to RAM content display
	raw.SetNormalInverseDisplay(0);					// Set Normal/Inverse Display = 0: Normal display
	raw.ChargePumpSetting(1);						// Charge Pump Settig = 1: Enable charge pump during display on
	raw.DeactivateScroll();							// Deactivate Scroll
	raw.SetDisplayOnOff(1);							// Set Display ON/OFF = 1: ON
}

void SSD1306::Refresh()
{
	raw.SetColumnAddress(0, GetWidth() - 1);
	raw.SetPageAddress(0, GetNumPages() - 1);
	raw.WriteBuffer();
}

template<class Logic> void SSD1306::DrawHLineT_NoAdjust(int x, int y, int width)
{
	uint8_t data = 0b00000001 << (y & 0b111);
	uint8_t* p = raw.GetPointer(x, y);
	for (int i = 0; i < width; i++, p++) {
		assert(raw.EnsureSafePointer(p));
		*p = Logic()(*p, data);
	}
}

template<class Logic> void SSD1306::DrawVLineT_NoAdjust(int x, int y, int height)
{
	uint64_t bits = (-1LL << y) & ~(-1LL << (y + height));
	int page;
	uint8_t* pTop = raw.GetPointer(x, y, &page);
	bits >>= page * 8;
	for (uint8_t* p = pTop; page < GetNumPages() && bits; page++, p += GetWidth(), bits >>= 8) {
		assert(raw.EnsureSafePointer(p));
		*p = Logic()(*p, static_cast<uint8_t>(bits & 0b11111111));
	}
}

template<class Logic> void SSD1306::DrawHLineT(int x, int y, int width)
{
	if (!CheckCoord(y, GetHeight()) || !AdjustCoord(&x, &width, GetWidth())) return;
	DrawHLineT_NoAdjust<Logic>(x, y, width);
}

template<class Logic> void SSD1306::DrawVLineT(int x, int y, int height)
{
	if (!CheckCoord(x, GetWidth()) || !AdjustCoord(&y, &height, GetHeight())) return;
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
	if (AdjustCoord(&xLeftAdjust, &widthAdjust, GetWidth())) {
		if (CheckCoord(yTop, GetHeight())) {
			DrawHLineT_NoAdjust<Logic>(xLeftAdjust, yTop, widthAdjust);
		}
		if (CheckCoord(yBottom, GetHeight())) {
			DrawHLineT_NoAdjust<Logic>(xLeftAdjust, yBottom, widthAdjust);
		}
	}
	if (AdjustCoord(&yTopAdjust, &heightAdjust, GetHeight())) {
		if (CheckCoord(xLeft, GetWidth())) {
			DrawVLineT_NoAdjust<Logic>(xLeft, yTopAdjust, heightAdjust);
		}
		if (CheckCoord(xRight, GetWidth())) {
			DrawVLineT_NoAdjust<Logic>(xRight, yTopAdjust, heightAdjust);
		}
	}
}

template<class Logic> void SSD1306::DrawRectFillT(int x, int y, int width, int height)
{
	if (!AdjustCoord(&x, &width, GetWidth()) || !AdjustCoord(&y, &height, GetHeight())) return;
	uint64_t bits = (-1LL << y) & ~(-1LL << (y + height));
	int page;
	uint8_t* pTop = raw.GetPointer(x, y, &page);
	bits >>= page * 8;
	for ( ; page < GetNumPages() && bits; page++, pTop += GetWidth(), bits >>= 8) {
		uint8_t* p = pTop;
		for (int i = 0; i < width; i++, p++) {
			assert(raw.EnsureSafePointer(p));
			*p = Logic()(*p, static_cast<uint8_t>(bits & 0b11111111));
		}
	}
}

template<class Logic> void SSD1306::DrawCharT(int x, int y, const FontEntry* pFontEntry)
{
	if (!pFontEntry) return;
	int wdFont = pFontEntry->width;
	int htFont = pFontEntry->height;
	int bytesPerLine = (wdFont + 7) / 8;
	int width = wdFont * fontScaleX_;
	int height = htFont * fontScaleY_;
	if (!AdjustCoord(&x, &width, GetWidth()) || !AdjustCoord(&y, &height, GetHeight())) return;
	const uint8_t* pDataBottom = pFontEntry->data + bytesPerLine * htFont;
	int pageTop;
	uint8_t* pTop = raw.GetPointer(x, y, &pageTop);
	int bitOffset = y - pageTop * 8;
	int xCur = x;
	uint32_t bitsDot = (1 << fontScaleY_) - 1;
	for (int i = 0; i < bytesPerLine; i++, pDataBottom++) {
		for (uint8_t bitMask = 0x80; bitMask; bitMask >>= 1, pTop += fontScaleX_, xCur += fontScaleX_) {
			uint64_t bits = 0;
			const uint8_t* pData = pDataBottom;
			for (int j = 0; j < htFont; j++) {
				pData -= bytesPerLine;
				bits = (bits << fontScaleY_) | ((*pData & bitMask)? bitsDot : 0);
			}
			bits <<= bitOffset;
			uint8_t* pLeft = pTop;
			for (int page = pageTop; page < GetNumPages() && bits; page++, pLeft += GetWidth(), bits >>= 8) {
				uint8_t* p = pLeft;
				uint8_t data = static_cast<uint8_t>(bits & 0b11111111);
				for (int j = 0; j < fontScaleX_ && xCur + j < GetWidth(); j++, p++) {
					assert(raw.EnsureSafePointer(p));
					*p = Logic()(*p, data);
				}
			}
		}
	}
}

template<class Logic> void SSD1306::DrawCharT(int x, int y, uint32_t code)
{
	const FontEntry* pFontEntry = pFontSetCur_->GetFontEntry(code);
	DrawCharT<Logic>(x, y, pFontEntry);
}

template<class Logic> void SSD1306::DrawStringT(int x, int y, const char* str)
{
	uint32_t code;
	UTF8Decoder decoder;
	for (const char* p = str; *p; p++) {
		if (decoder.FeedChar(*p, &code)) {
			const FontEntry* pFontEntry = pFontSetCur_->GetFontEntry(code);
			DrawCharT<Logic>(x, y, pFontEntry);
			x += pFontEntry->width * fontScaleX_;
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

void SSD1306::DrawChar(int x, int y, uint32_t code)
{
	DrawCharT<Logic_Draw>(x, y, code);
}

void SSD1306::DrawString(int x, int y, const char* str)
{
	DrawStringT<Logic_Draw>(x, y, str);
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

void SSD1306::EraseChar(int x, int y, uint32_t code)
{
	DrawCharT<Logic_Erase>(x, y, code);
}

void SSD1306::EraseString(int x, int y, const char* str)
{
	DrawStringT<Logic_Erase>(x, y, str);
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

void SSD1306::InvertChar(int x, int y, uint32_t code)
{
	DrawCharT<Logic_Invert>(x, y, code);
}

void SSD1306::InvertString(int x, int y, const char* str)
{
	DrawStringT<Logic_Invert>(x, y, str);
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
