//==============================================================================
// 128x64 Dot Matrix
// OLED/PLED Segment/Common Driver with Controller
//==============================================================================
#ifndef SSD1306_H
#define SSD1306_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

class SSD1306 {
public:
	static const int DisplayWidth = 128;
	static const int DisplayHeight = 64;
	static const int PageHeight = 8;
	static const int NumPages = DisplayHeight / PageHeight;
	static const int BufferLen = NumPages * DisplayWidth;
public:
	class Raw {
	private:
		uint8_t addr_;
		uint8_t* buffWhole_;
		uint8_t* buff_;
	public:
		Raw(uint8_t addr) : addr_(addr), buffWhole_(nullptr), buff_(nullptr) {}
		~Raw() {
			::free(buffWhole_);
		}
		void AllocBuff() {
			buffWhole_ = reinterpret_cast<uint8_t*>(::malloc(BufferLen + 1));
			buffWhole_[0] = 
				(0b0 << 7) |	// Co = 0
				(0b1 << 6);		// D/C# = 1
			buff_ = buffWhole_ + 1;
			FillBuffer(0x00);
		}
		uint8_t GetAddr() const { return addr_; }
		void WriteCtrl(uint8_t ctrl) const {
			uint8_t buff[2];
			buff[0] =
				(0b1 << 7) |	// Co = 1
				(0b0 << 6);		// D/C# = 0
			buff[1] = ctrl;
			::i2c_write_blocking(i2c_default, addr_, buff, sizeof(buff), false);
		}
		uint8_t* GetPointer() { return buff_; }
		uint8_t* GetPointer(int x) { return buff_ + x; }
		uint8_t* GetPointer(int x, int y) { return buff_ + (y / 8) * DisplayWidth + x; }
		uint8_t* GetPointer(int x, int y, int* pPage) { *pPage = y / 8; return buff_ + *pPage * DisplayWidth + x; }
		void FillBuffer(uint8_t data) { ::memset(buff_, data, BufferLen); }
		void WriteBuffer() const {
			::i2c_write_blocking(i2c_default, addr_, buffWhole_, BufferLen + 1, false);
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
	Raw raw;
public:
	SSD1306(uint8_t addr = 0x3c) : raw(addr) {}
	void Initialize() {
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
	void Refresh() {
		raw.SetColumnAddress(0, DisplayWidth - 1);
		raw.SetPageAddress(0, NumPages - 1);
		raw.WriteBuffer();
	}
	void Flash(bool flashFlag) { raw.EntireDisplayOn(static_cast<uint8_t>(flashFlag)); }
	void Clear() { raw.FillBuffer(0x00); }
	void DrawPixel(int x, int y) { *raw.GetPointer(x, y) |= 1 << (y & 0b111); }
	void ErasePixel(int x, int y) { *raw.GetPointer(x, y) &= ~(1 << (y & 0b111)); }
	void DrawHLine(int x, int y, int width);
	void EraseHLine(int x, int y, int width);
	void DrawVLine(int x, int y, int height);
	void EraseVLine(int x, int y, int height);
	void DrawLine(int x0, int y0, int x1, int y1);
	void EraseLine(int x0, int y0, int x1, int y1);
private:
	static void SortPair(int v1, int v2, int* pMin, int* pMax);
	static bool CheckCoord(int v, int vLimit) { return 0 <= v && v < vLimit; }
	static bool AdjustCoord(int* pV, int* pDist, int vLimit);
};

#endif
