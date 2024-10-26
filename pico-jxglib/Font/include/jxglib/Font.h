//==============================================================================
// Font
//==============================================================================
#ifndef FONT_H
#define FONT_H
#include <pico/stdlib.h>

struct FontEntry {
	uint32_t code;
	int width;
	int height;
	uint8_t data[];
};

struct FontSet {
	const FontEntry* GetFontEntry(uint32_t code) const;
	const FontEntry* pFontEntry_Invalid;
	const FontEntry* pFontEntryTbl_Basic[96];
	int nFontEntries_Extra;
	const FontEntry* pFontEntries_Extra[];
};

#endif
