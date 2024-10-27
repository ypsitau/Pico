//==============================================================================
// jxglib/SSD1306.h
// 128x64 Dot Matrix OLED/PLED Segment/Common Driver with Controller
// Specification: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
//==============================================================================
#ifndef PICO_JXGLIB_SSD1306_H
#define PICO_JXGLIB_SSD1306_H
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "jxglib/Common.h"
#include "jxglib/Font.h"

//------------------------------------------------------------------------------
// SSD1306
//------------------------------------------------------------------------------
class SSD1306 {
public:
	class Logic_Draw {
	public:
		uint8_t operator()(uint8_t v1, uint8_t v2) { return v1 | v2; }
	};
	class Logic_Erase {
	public:
		uint8_t operator()(uint8_t v1, uint8_t v2) { return v1 & ~v2; }
	};
	class Logic_Invert {
	public:
		uint8_t operator()(uint8_t v1, uint8_t v2) { return v1 ^ v2; }
	};
	class Raw {
	private:
		static const int width_ = 128;
		int height_;
		static const int heightPerPage_ = 8;
		int numPages_;
		int bufferLen_;
	private:
		i2c_inst_t* i2c_;
		uint8_t addr_;
		uint8_t* buffWhole_;
		uint8_t* buff_;
	public:
		Raw(i2c_inst_t* i2c, uint8_t addr, int height) :
				i2c_(i2c), addr_(addr), buffWhole_(nullptr), buff_(nullptr), height_(height) {
			numPages_ = height_ / heightPerPage_;
			bufferLen_ = numPages_ * width_;
		}
		~Raw() {
			::free(buffWhole_);
		}
	public:
		uint8_t GetAddr() const { return addr_; }
		int GetWidth() const { return width_; }
		int GetHeight() const { return height_; }
		int GetHeightPerPage() const { return heightPerPage_; }
		int GetNumPages() const { return numPages_; }
		int GetBufferLen() const { return bufferLen_; }
	public:
		void AllocBuffer() {
			buffWhole_ = reinterpret_cast<uint8_t*>(::malloc(bufferLen_ + 1));
			buffWhole_[0] = 
				(0b0 << 7) |	// Co = 0
				(0b1 << 6);		// D/C# = 1
			buff_ = buffWhole_ + 1;
			FillBuffer(0x00);
		}
		void WriteCtrl(uint8_t ctrl) const {
			uint8_t buff[2];
			buff[0] =
				(0b1 << 7) |	// Co = 1
				(0b0 << 6);		// D/C# = 0
			buff[1] = ctrl;
			::i2c_write_blocking(i2c_, addr_, buff, sizeof(buff), false);
		}
		uint8_t* GetPointer() { return buff_; }
		uint8_t* GetPointer(int x) { return buff_ + x; }
		uint8_t* GetPointer(int x, int y) { return buff_ + (y / 8) * width_ + x; }
		uint8_t* GetPointer(int x, int y, int* pPage) { *pPage = y / 8; return buff_ + *pPage * width_ + x; }
		bool EnsureSafePointer(const uint8_t* p) { return buff_ <= p && p < buff_ + bufferLen_; }
		void FillBuffer(uint8_t data) { ::memset(buff_, data, bufferLen_); }
		void WriteBuffer() const {
			::i2c_write_blocking(i2c_, addr_, buffWhole_, bufferLen_ + 1, false);
		}
public:
		// 10.1 Fundamental Command
		// 10.1.1 Set Lower Column Start Address for Page Addressing Mode (00h-0Fh)
		// 10.1.2 Set Higher Column Start Address for Page Addressing Mode (10h-1Fh)
		void SetColumnStartAddressForPageAddressingMode(uint8_t columnStartAddr) const {
			WriteCtrl(0x00 | (columnStartAddr & 0x0f));
			WriteCtrl(0x10 | (columnStartAddr >> 4));
		}
		// 10.1.3 Set Memory Addressing Mode (20h)
		void SetMemoryAddressingMode(uint8_t mode) const {
			WriteCtrl(0x20);
			WriteCtrl(mode);
		}
		// 10.1.4 Set Column Address (21h)
		void SetColumnAddress(uint8_t columnStartAddr, uint8_t columnEndAddr) const {
			WriteCtrl(0x21);
			WriteCtrl(columnStartAddr);
			WriteCtrl(columnEndAddr);
		}
		// 10.1.5 Set Page Address (22h)
		void SetPageAddress(uint8_t pageStartAddr, uint8_t pageEndAddr) const {
			WriteCtrl(0x22);
			WriteCtrl(pageStartAddr);
			WriteCtrl(pageEndAddr);
		}
		// 10.1.6 Set Display Start Line (40h-7Fh)
		void SetDisplayStartLine(uint8_t startLine) const {
			WriteCtrl(0x40 | startLine);
		}
		// 10.1.7 Set Contrast Control for BANK0 (81h)
		void SetContrastControl(uint8_t contrast) const {
			WriteCtrl(0x81);
			WriteCtrl(contrast);
		}
		// 10.1.8 SetSegmentRe-map (A0h/A1h)
		void SetSegmentRemap(uint8_t flag) const {
			WriteCtrl(0xa0 | flag);
		}
		// 10.1.9 Entire Display ON (A4h/A5h)
		void EntireDisplayOn(uint8_t allOnFlag) const {
			WriteCtrl(0xa4 | allOnFlag);
		}
		// 10.1.10 Set Normal/Inverse Display (A6h/A7h)
		void SetNormalInverseDisplay(uint8_t inverse) const {
			WriteCtrl(0xa6 | inverse);
		}
		// 10.1.11 Set Multiplex Ratio (A8h)
		void SetMultiplexRatio(uint8_t ratio) const {
			WriteCtrl(0xa8);
			WriteCtrl(ratio);
		}
		// 10.1.12 Set Display ON/OFF (AEh/AFh)
		void SetDisplayOnOff(uint8_t on) const {
			WriteCtrl(0xae | on);
		}
		// 10.1.13 Set Page Start Address for Page Addressing Mode (B0h-B7h)
		void SetPageStartAddressForPageAddressingMode(uint8_t pageStartAddrGDDRAM) const {
			WriteCtrl(0xb0 | pageStartAddrGDDRAM);
		}
		// 10.1.14 Set COM Output Scan Direction (C0h/C8h)
		void SetCOMOutputScanDirection(uint8_t scanDir) const {
			WriteCtrl(0xc0 | (scanDir << 3));
		}
		// 10.1.15 Set Display Offset (D3h)
		void SetDisplayOffset(uint8_t offset) const {
			WriteCtrl(0xd3);
			WriteCtrl(offset);
		}
		// 10.1.16 Set Display Clock Divide Ratio / Oscillator Frequency (D5h)
		void SetDisplayClockDivideRatioOscillatorFrequency(uint8_t divideRatio, uint8_t oscillatorFreq) const {
			WriteCtrl(0xd5);
			WriteCtrl((oscillatorFreq << 4) | divideRatio);
		}
		// 10.1.17 Set Pre-charge Period (D9h)
		void SetPrechargePeriod(uint8_t clocksPhase1, uint8_t clocksPhase2) const {
			WriteCtrl(0xd9);
			WriteCtrl((clocksPhase2 << 4) | clocksPhase1);
		}
		// 10.1.18 Set COM Pins Hardware Configuration (DAh)
		void SetCOMPinsHardwareConfiguration(uint8_t pinConfig, uint8_t enableRemap) const {
			WriteCtrl(0xda);
			WriteCtrl(0x02 | (enableRemap << 5) | (pinConfig << 4));
		}
		// 10.1.19 Set Vcomh Detect Level (DBh)
		void SetVcomhDeselectLevel(uint8_t level) const {
			WriteCtrl(0xdb);
			WriteCtrl(level << 4);
		}
		// 10.1.20 NOP (E3h)
		void NOP() const {
			WriteCtrl(0xe3);
		}
		// 10.2 Graphic Acceleration Command
		// 10.2.1 Horizontal Scroll Setup (26h/27h)
		void HorizontalScrollSetup(uint8_t left, uint8_t startPageAddr, uint8_t endPageAddr, uint8_t framesInterval) const {
			WriteCtrl(0x26 | left);
			WriteCtrl(0x00);				// Dummy byte
			WriteCtrl(startPageAddr);
			WriteCtrl(framesInterval);
			WriteCtrl(endPageAddr);
			WriteCtrl(0x00);				// Dummy byte
			WriteCtrl(0xff);				// Dummy byte
		}
		// 10.2.2 Continuous Vertical and Horizontal Scroll Setup (29h/2Ah)
		void ContinuousVerticalAndHorizontalScrollSetup(uint8_t left, uint8_t startPageAddr, uint8_t endPageAddr, uint8_t framesInterval, uint8_t verticalScrollingOffset) const {
			WriteCtrl(0x29 + left);
			WriteCtrl(0x00);				// Dummy byte
			WriteCtrl(startPageAddr);
			WriteCtrl(framesInterval);
			WriteCtrl(endPageAddr);
			WriteCtrl(verticalScrollingOffset);
		}
		// 10.2.3 Deactivate Scroll (2Eh)
		void DeactivateScroll() const {
			WriteCtrl(0x2e);
		}
		// 10.2.4 Activate Scroll (2Fh)
		void ActivateScroll() const {
			WriteCtrl(0x2f);
		}
		// 10.2.5 Set Vertical Scroll Area (A3h)
		void SetVerticalScrollArea(uint8_t rowsFixedArea, uint8_t rowsScrollArea) const {
			WriteCtrl(0xa3);
			WriteCtrl(rowsFixedArea);
			WriteCtrl(rowsScrollArea);
		}
		// Charge Pump Regulator
		void ChargePumpSetting(uint8_t enableChargePump) const {
			WriteCtrl(0x8d);
			WriteCtrl(0x10 | (enableChargePump << 2));
		}
	};
public:
	static const uint8_t DefaultAddr = 0x3c;
	Raw raw;
public:
	const FontSet* pFontSetCur_;
	int fontScaleX_, fontScaleY_;
public:
	SSD1306(i2c_inst_t* i2c, uint8_t addr = DefaultAddr, bool highResoFlag = true) :
			raw(i2c, addr, highResoFlag? 64 : 32), pFontSetCur_(nullptr), fontScaleX_(1), fontScaleY_(1) {}
public:
	uint8_t GetAddr() const { return raw.GetAddr(); }
	int GetWidth() const { return raw.GetWidth(); }
	int GetHeight() const { return raw.GetHeight(); }
	int GetHeightPerPage() const { return raw.GetHeightPerPage(); }
	int GetNumPages() const { return raw.GetNumPages(); }
	int GetBufferLen() const { return raw.GetBufferLen(); }
public:
	void Initialize();
	void Refresh();
	void Flash(bool flashFlag) { raw.EntireDisplayOn(static_cast<uint8_t>(flashFlag)); }
	void Clear(uint8_t data = 0x00) { raw.FillBuffer(data); }
	void SetFont(const FontSet& fontSet) { pFontSetCur_ = &fontSet, fontScaleX_ = fontScaleY_ = 1; }
	void SetFontScale(int fontScale) { fontScaleX_ = fontScaleY_ = fontScale; }
	void SetFontScale(int fontScaleX, int fontScaleY) { fontScaleX_ = fontScaleX, fontScaleY_ = fontScaleY; }
private:
	template<class Logic> void DrawPixelT(int x, int y) {
		uint8_t* p = raw.GetPointer(x, y);
		*p = Logic()(*p, 1 << (y & 0b111));
	}
	template<class Logic> void DrawHLineT_NoAdjust(int x, int y, int width);
	template<class Logic> void DrawVLineT_NoAdjust(int x, int y, int height);
	template<class Logic> void DrawHLineT(int x, int y, int width);
	template<class Logic> void DrawVLineT(int x, int y, int width);
	template<class Logic> void DrawLineT(int x0, int y0, int x1, int y1);
	template<class Logic> void DrawRectT(int x, int y, int width, int height);
	template<class Logic> void DrawRectFillT(int x, int y, int width, int height);
	template<class Logic> void DrawCharT(int x, int y, const FontEntry* pFontEntry);
	template<class Logic> void DrawCharT(int x, int y, uint32_t code);
	template<class Logic> void DrawStringT(int x, int y, const char* str);
public:
	// Draw* Method
	void DrawPixel(int x, int y) { DrawPixelT<Logic_Draw>(x, y); }
	void DrawPixel(const Point& pt) { DrawPixel(pt.x, pt.y); }
	void DrawHLine(int x, int y, int width);
	void DrawHLine(const Point& pt, int width) { DrawHLine(pt.x, pt.y, width); }
	void DrawVLine(int x, int y, int height);
	void DrawVLine(const Point& pt, int height) { DrawVLine(pt.x, pt.y, height); }
	void DrawLine(int x0, int y0, int x1, int y1);
	void DrawLine(const Point& pt1, const Point& pt2) { DrawLine(pt1.x, pt1.y, pt2.x, pt2.y); }
	void DrawRect(int x, int y, int width, int height);
	void DrawRect(const Rect& rc) { DrawRect(rc.x, rc.y, rc.width, rc.height); }
	void DrawRectFill(int x, int y, int width, int height);
	void DrawRectFill(const Rect& rc) { DrawRect(rc.x, rc.y, rc.width, rc.height); }
	void DrawChar(int x, int y, uint32_t code);
	void DrawChar(const Point& pt, uint32_t code) { DrawChar(pt.x, pt.y, code); }
	void DrawString(int x, int y, const char* str);
	void DrawString(const Point& pt, const char* str) { DrawString(pt.x, pt.y, str); }
	// Erase* Method
	void ErasePixel(int x, int y) { DrawPixelT<Logic_Erase>(x, y); }
	void ErasePixel(const Point& pt) { ErasePixel(pt.x, pt.y); }
	void EraseHLine(int x, int y, int width);
	void EraseHLine(const Point& pt, int width) { EraseHLine(pt.x, pt.y, width); }
	void EraseVLine(int x, int y, int height);
	void EraseVLine(const Point& pt, int height) { EraseVLine(pt.x, pt.y, height); }
	void EraseLine(int x0, int y0, int x1, int y1);
	void EraseLine(const Point& pt1, const Point& pt2) { EraseLine(pt1.x, pt1.y, pt2.x, pt2.y); }
	void EraseRect(int x, int y, int width, int height);
	void EraseRect(const Rect& rc) { EraseRect(rc.x, rc.y, rc.width, rc.height); }
	void EraseRectFill(int x, int y, int width, int height);
	void EraseRectFill(const Rect& rc) { EraseRect(rc.x, rc.y, rc.width, rc.height); }
	void EraseChar(int x, int y, uint32_t code);
	void EraseChar(const Point& pt, uint32_t code) { EraseChar(pt.x, pt.y, code); }
	void EraseString(int x, int y, const char* str);
	void EraseString(const Point& pt, const char* str) { EraseString(pt.x, pt.y, str); }
	// Invert* Method
	void InvertPixel(int x, int y) { DrawPixelT<Logic_Invert>(x, y); }
	void InvertPixel(const Point& pt) { InvertPixel(pt.x, pt.y); }
	void InvertHLine(int x, int y, int width);
	void InvertHLine(const Point& pt, int width) { InvertHLine(pt.x, pt.y, width); }
	void InvertVLine(int x, int y, int height);
	void InvertVLine(const Point& pt, int height) { InvertVLine(pt.x, pt.y, height); }
	void InvertLine(int x0, int y0, int x1, int y1);
	void InvertLine(const Point& pt1, const Point& pt2) { InvertLine(pt1.x, pt1.y, pt2.x, pt2.y); }
	void InvertRect(int x, int y, int width, int height);
	void InvertRect(const Rect& rc) { InvertRect(rc.x, rc.y, rc.width, rc.height); }
	void InvertRectFill(int x, int y, int width, int height);
	void InvertRectFill(const Rect& rc) { InvertRect(rc.x, rc.y, rc.width, rc.height); }
	void InvertChar(int x, int y, uint32_t code);
	void InvertChar(const Point& pt, uint32_t code) { InvertChar(pt.x, pt.y, code); }
	void InvertString(int x, int y, const char* str);
	void InvertString(const Point& pt, const char* str) { InvertString(pt.x, pt.y, str); }
private:
	static void SortPair(int v1, int v2, int* pMin, int* pMax);
	static bool CheckCoord(int v, int vLimit) { return 0 <= v && v < vLimit; }
	static bool AdjustCoord(int* pV, int* pDist, int vLimit);
};

#endif
