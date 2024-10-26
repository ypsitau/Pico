#include "jxglib/UTF8Decoder.h"

//------------------------------------------------------------------------------
// UTF8Decoder
//------------------------------------------------------------------------------
bool UTF8Decoder::FeedChar(char ch, uint32_t* pCodeUTF32)
{
	uint8_t chCasted = static_cast<uint8_t>(ch);
	if ((chCasted & 0x80) == 0x00) {
		*pCodeUTF32 = chCasted;
		return true;
	} else if (nFollowers_ > 0) {
		if ((chCasted & 0xc0) == 0x80) {
			codeUTF32_ = (codeUTF32_ << 6) | (chCasted & 0x3f);
		} else {
			codeUTF32_ <<= 6;
		}
		nFollowers_--;
		if (nFollowers_ == 0) {
			*pCodeUTF32 = codeUTF32_;
			return true;
		}
	} else if ((chCasted & 0xe0) == 0xc0) {
		codeUTF32_ = chCasted & 0x1f;
		nFollowers_ = 1;
	} else if ((chCasted & 0xf0) == 0xe0) {
		codeUTF32_ = chCasted & 0x0f;
		nFollowers_ = 2;
	} else if ((chCasted & 0xf8) == 0xf0) {
		codeUTF32_ = chCasted & 0x07;
		nFollowers_ = 3;
	} else if ((chCasted & 0xfc) == 0xf8) {
		codeUTF32_ = chCasted & 0x03;
		nFollowers_ = 4;
	} else {
		codeUTF32_ = chCasted & 0x01;
		nFollowers_ = 5;
	}
	return false;
}
