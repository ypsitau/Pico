//==============================================================================
// jxglib/Font.h
//==============================================================================
#ifndef PICO_JXGLIB_FONT_H
#define PICO_JXGLIB_FONT_H
#include "pico/stdlib.h"

//------------------------------------------------------------------------------
// FontEntry
//------------------------------------------------------------------------------
struct FontEntry {
	uint32_t code;
	int width;
	int height;
	int xAdvance;
	uint8_t data[];
};

//------------------------------------------------------------------------------
// FontSet
//------------------------------------------------------------------------------
struct FontSet {
	const FontEntry* GetFontEntry(uint32_t code) const;
	const FontEntry* pFontEntry_Invalid;
	const FontEntry* pFontEntryTbl_Basic[96];
	int nFontEntries_Extra;
	const FontEntry* pFontEntries_Extra[];
};

#endif
