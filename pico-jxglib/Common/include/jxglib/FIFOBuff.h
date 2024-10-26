//==============================================================================
// jxglib/FIFOBuff.h
//==============================================================================
#ifndef PICO_JXGLIB_FIFOBUFF_H
#define PICO_JXGLIB_FIFOBUFF_H
#include <stdio.h>
#include "pico/stdlib.h"

//------------------------------------------------------------------------------
// FIFOBuff
//------------------------------------------------------------------------------
template<typename T_Elem, int buffSize, typename T_Size = uint8_t> class FIFOBuff {
public:
	static constexpr T_Size sizeMinusOne = static_cast<T_Size>(buffSize - 1);
private:
	volatile T_Size posRead_;
	volatile T_Size posRewind_;
	volatile T_Size posWrite_;
	volatile T_Elem buff_[buffSize];
public:
	FIFOBuff() : posRead_(0), posRewind_(0), posWrite_(0) {}
	void Clear() { posRead_ = posRewind_ = posWrite_ = 0; }
	bool IsEmpty() const { return posRead_ == posWrite_; }
	bool HasData() const { return posRead_ != posWrite_; }
	T_Size GetReadPos() const { return posRead_; }
	T_Size GetWritePos() const { return posWrite_; }
	void SetRewindPosToWritePos() { posRewind_ = posWrite_; }
	void RewindReadPos() { posRead_ = posRewind_; }
	bool IsFull() const {
		T_Size posWriteNext = (posWrite_ == sizeMinusOne)? 0 : posWrite_ + 1;
		return posWriteNext == posRead_;
	}
	bool HasRoom() const { return !IsFull(); }
	T_Size GetLength() const {
		return (posRead_ <= posWrite_)? posWrite_ - posRead_ : buffSize - posRead_ + posWrite_;
	}
	void WriteData(T_Elem data) {
		T_Size posWriteNext = (posWrite_ == sizeMinusOne)? 0 : posWrite_ + 1;
		if (posWriteNext == posRead_) return;
		buff_[posWrite_] = data;
		posWrite_ = posWriteNext;
	}
	T_Elem ReadData() {
		//if (posRead_ == posWrite_) return 0;
		T_Elem data = buff_[posRead_];
		posRead_ = (posRead_ == sizeMinusOne)? 0 : posRead_ + 1;
		return data;
	}
	void WriteBuff(const T_Elem* buff, T_Size len) {
		for ( ; len > 0; len--, buff++) WriteData(*buff);
	}
	void ReadBuff(T_Elem* buff, T_Size len, T_Size* pLenRead = nullptr) {
		T_Elem* p = buff;
		for ( ; len > 0 && HasData(); len--, p++) *p = ReadData();
		if (pLenRead) *pLenRead = p - buff;
	}
};

#endif
