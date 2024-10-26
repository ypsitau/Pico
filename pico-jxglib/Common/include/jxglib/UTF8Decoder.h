#ifndef UTF8DECODER_H
#define UTF8DECODER_H
#include "pico/stdlib.h"

//------------------------------------------------------------------------------
// UTF8Decoder
//------------------------------------------------------------------------------
class UTF8Decoder {
private:
	int nFollowers_;
	uint32_t codeUTF32_;
public:
	UTF8Decoder() : nFollowers_(0), codeUTF32_(0) {}
	bool FeedChar(char ch, uint32_t* pCodeUTF32);
};
#endif
