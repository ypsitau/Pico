#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <hardware/clocks.h>
#include <hardware/i2c.h>
#include "SSD1306.h"

// 128x64 Dot Matrix
// OLED/PLED Segment/Common Driver with Controller
class SSD1306 {
public:
	static const int Width = 128;
	static const int Height = 32;
	static const int PageHeight = 8;
	static const int NumPages = Height / PageHeight;
	static const int BitPatternLen = NumPages * Width;
public:
	class Raw {
	private:
		uint8_t addr_;
		uint8_t* bitPatternBuff_;
		uint8_t* bitPattern_;
	public:
		Raw(uint8_t addr) : addr_(addr), bitPatternBuff_(nullptr), bitPattern_(nullptr) {}
		~Raw() {
			::free(bitPatternBuff_);
		}
		void AllocBuff() {
			bitPatternBuff_ = reinterpret_cast<uint8_t*>(::malloc(BitPatternLen + 1));
			bitPatternBuff_[0] = 
				(0b0 << 7) |	// Co = 0
				(0b1 << 6);		// D/C# = 1
			bitPattern_ = bitPatternBuff_ + 1;
			ClearBitPattern();
		}
		uint8_t GetAddr() const { return addr_; }
		void WriteCtrl(uint8_t ctrl) const {
#if 0
			printf("%02x ", ctrl);
#else
			uint8_t buff[2];
			buff[0] =
				(0b1 << 7) |	// Co = 1
				(0b0 << 6);		// D/C# = 0
			buff[1] = ctrl;
			::i2c_write_blocking(i2c_default, addr_, buff, sizeof(buff), false);
#endif
		}
		uint8_t* GetBitPattern() { return bitPattern_; }
		void ClearBitPattern() {
			::memset(bitPattern_, 0x00, BitPatternLen);
		}
		void WriteBitPattern() const {
			::i2c_write_blocking(i2c_default, addr_, bitPatternBuff_, BitPatternLen + 1, false);
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
		raw.SetMultiplexRatio(Height - 1);	// set multiplex ratio: Display height - 1
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
		raw.SetColumnAddress(0, Width - 1);
		raw.SetPageAddress(0, NumPages - 1);
		raw.WriteBitPattern();
	}
	void SetPixel(int x, int y, bool on);
	void DrawLine(int x0, int y0, int x1, int y1, bool on);
};

void SSD1306::SetPixel(int x, int y, bool on)
{
	//assert(x >= 0 && x < SSD1306_WIDTH && y >=0 && y < SSD1306_HEIGHT);

	// The calculation to determine the correct bit to set depends on which address
	// mode we are in. This code assumes horizontal

	// The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
	// Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
	// byte 1 is x = 1, y=0->7 etc

	// This code could be optimised, but is like this for clarity. The compiler
	// should do a half decent job optimising it anyway.

	uint8_t* buf = raw.GetBitPattern();
	int byte_idx = (y / 8) * Width + x;
	uint8_t byte = buf[byte_idx];

	if (on)
		byte |=  1 << (y % 8);
	else
		byte &= ~(1 << (y % 8));

	buf[byte_idx] = byte;
}

void SSD1306::DrawLine(int x0, int y0, int x1, int y1, bool on)
{

	int dx =  abs(x1-x0);
	int sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1-y0);
	int sy = y0<y1 ? 1 : -1;
	int err = dx+dy;
	int e2;

	while (true) {
		SetPixel(x0, y0, on);
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

#define ArraySizeOf(x) (sizeof(x) / sizeof(x[0]))

#define SSD1306_HEIGHT              32
#define SSD1306_WIDTH               128

#define SSD1306_PAGE_HEIGHT         _u(8)
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

#define SSD1306_SET_MEM_MODE        _u(0x20)
#define SSD1306_SET_COL_ADDR        _u(0x21)
#define SSD1306_SET_PAGE_ADDR       _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL    _u(0x26)
#define SSD1306_SET_SCROLL          _u(0x2E)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST        _u(0x81)
#define SSD1306_SET_CHARGE_PUMP     _u(0x8D)

#define SSD1306_SET_SEG_REMAP       _u(0xA0)
#define SSD1306_SET_ENTIRE_ON       _u(0xA4)
#define SSD1306_SET_ALL_ON          _u(0xA5)
#define SSD1306_SET_NORM_DISP       _u(0xA6)
#define SSD1306_SET_INV_DISP        _u(0xA7)
#define SSD1306_SET_MUX_RATIO       _u(0xA8)
#define SSD1306_SET_DISP            _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR     _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET     _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    _u(0xD5)
#define SSD1306_SET_PRECHARGE       _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG     _u(0xDA)
#define SSD1306_SET_VCOM_DESEL      _u(0xDB)

#define SSD1306_PAGE_HEIGHT         _u(8)
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

#define SSD1306_WRITE_MODE         _u(0xFE)
#define SSD1306_READ_MODE          _u(0xFF)

void SSD1306_send_cmd(uint8_t cmd)
{
	uint8_t addr = 0x3c;
	// I2C write process expects a control byte followed by data
	// this "data" can be a command or data to follow up a command
	// Co = 1, D/C = 0 => the driver expects a command
	uint8_t buf[2] = {0x80, cmd};
	::i2c_write_blocking(i2c_default, addr, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num)
{
	for (int i=0;i<num;i++)
		SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen)
{
	uint8_t addr = 0x3c;
	// in horizontal addressing mode, the column address pointer auto-increments
	// and then wraps around to the next page, so we can send the entire frame
	// buffer in one gooooooo!

	// copy our frame buffer into a new buffer because we need to add the control byte
	// to the beginning

	uint8_t *temp_buf = reinterpret_cast<uint8_t*>(malloc(buflen + 1));

	temp_buf[0] = 0x40;
	memcpy(temp_buf+1, buf, buflen);

	i2c_write_blocking(i2c_default, addr, temp_buf, buflen + 1, false);

	free(temp_buf);
}

void render(uint8_t *buf)
{
	// update a portion of the display with a render area
	uint8_t cmds[] = {
		SSD1306_SET_COL_ADDR,
		0,
		SSD1306_WIDTH - 1,
		SSD1306_SET_PAGE_ADDR,
		0,
		SSD1306_NUM_PAGES - 1
	};
	
	SSD1306_send_cmd_list(cmds, count_of(cmds));
	SSD1306_send_buf(buf, SSD1306_BUF_LEN);
}

static void SetPixel(uint8_t *buf, int x,int y, bool on)
{
	assert(x >= 0 && x < SSD1306_WIDTH && y >=0 && y < SSD1306_HEIGHT);

	// The calculation to determine the correct bit to set depends on which address
	// mode we are in. This code assumes horizontal

	// The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
	// Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
	// byte 1 is x = 1, y=0->7 etc

	// This code could be optimised, but is like this for clarity. The compiler
	// should do a half decent job optimising it anyway.

	const int BytesPerRow = SSD1306_WIDTH ; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

	int byte_idx = (y / 8) * BytesPerRow + x;
	uint8_t byte = buf[byte_idx];

	if (on)
		byte |=  1 << (y % 8);
	else
		byte &= ~(1 << (y % 8));

	buf[byte_idx] = byte;
}

static void DrawLine(uint8_t *buf, int x0, int y0, int x1, int y1, bool on)
{

	int dx =  abs(x1-x0);
	int sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1-y0);
	int sy = y0<y1 ? 1 : -1;
	int err = dx+dy;
	int e2;

	while (true) {
		SetPixel(buf, x0, y0, on);
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

int main()
{
	::stdio_init_all();
	::i2c_init(i2c_default, 400000);
	::gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	::gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	::gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	::gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
#if 0
	uint8_t cmds[] = {
		SSD1306_SET_DISP,               // set display off
		/* memory mapping */
		SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
		0x00,                           // horizontal addressing mode
		/* resolution and layout */
		SSD1306_SET_DISP_START_LINE,    // set display start line to 0
		SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
		SSD1306_SET_MUX_RATIO,          // set multiplex ratio
		SSD1306_HEIGHT - 1,             // Display height - 1
		SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
		SSD1306_SET_DISP_OFFSET,        // set display offset
		0x00,                           // no offset
		SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number. 
										// 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
		0x02,                           
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
		0x12,
#else
		0x02,
#endif
		/* timing and driving scheme */
		SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
		0x80,                           // div ratio of 1, standard freq
		SSD1306_SET_PRECHARGE,          // set pre-charge period
		0xF1,                           // Vcc internally generated on our board
		SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
		0x30,                           // 0.83xVcc
		/* display */
		SSD1306_SET_CONTRAST,           // set contrast control
		0xFF,
		SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
		SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
		SSD1306_SET_CHARGE_PUMP,        // set charge pump
		0x14,                           // Vcc internally generated on our board
		SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
		SSD1306_SET_DISP | 0x01, // turn display on
	};
	printf("----\n");
	for (int i = 0; i < ArraySizeOf(cmds); i++) {
		printf("%02x ", cmds[i]);
	}
	for (int i = 0; i < ArraySizeOf(cmds); i++) {
		SSD1306_send_cmd(cmds[i]);
	}
	printf("\n");
#endif
	SSD1306 oled;
	oled.Initialize();
	oled.Refresh();
	for (int i = 0; i < 3; i++) {
		oled.raw.EntireDisplayOn(1);
		::sleep_ms(500);
		oled.raw.EntireDisplayOn(0);
		::sleep_ms(500);
	}
	oled.DrawLine(0, 0, 100, 30, true);
	oled.Refresh();
#if 0
	// zero the entire display
	uint8_t buf[SSD1306_BUF_LEN];
	memset(buf, 0, SSD1306_BUF_LEN);
	render(buf);
	for (int i = 0; i < 3; i++) {
		SSD1306_send_cmd(SSD1306_SET_ALL_ON);    // Set all pixels on
		sleep_ms(500);
		SSD1306_send_cmd(SSD1306_SET_ENTIRE_ON); // go back to following RAM for pixel state
		sleep_ms(500);
	}
	DrawLine(buf, 0, 0, 100, 30, true);
	render(buf);
#endif
	for (;;) ;
}
