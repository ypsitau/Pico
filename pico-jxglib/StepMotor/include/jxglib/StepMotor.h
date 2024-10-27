//==============================================================================
// jxglib/StepMotor.h
//==============================================================================
#ifndef PICO_JXGLIB_STEPMOTOR_H
#define PICO_JXGLIB_STEPMOTOR_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

//------------------------------------------------------------------------------
// StepMotor
//------------------------------------------------------------------------------
class StepMotor {
public:
	enum class Status { Stop, Running };
	enum class Direction { A, B };
private:
	uint idxSm_;
	int gpioFirst_;
	int nPulsesPerSec_;
	int posCur_;
	int nPulsesToSet_;
	Status status_;
	Direction direction_;
private:
	static PIO pio_;
	static int offsetProgram_;
public:
	const uint32_t Pattern_Full_A = 0b1100'0110'0011'1001'1100'0110'0011'1001;
	const uint32_t Pattern_Full_B = 0b1100'1001'0011'0110'1100'1001'0011'0110;
	const uint32_t Pattern_Half_A = 0b1100'1000'1001'0001'0011'0010'0110'0100;
	const uint32_t Pattern_Half_B = 0b1100'0100'0110'0010'0011'0001'1001'1000;
public:
	StepMotor(int gpioFirst, int nPulsesPerSec = 500) :
		idxSm_(static_cast<uint>(-1)), gpioFirst_(gpioFirst), nPulsesPerSec_(nPulsesPerSec),
		posCur_(0), nPulsesToSet_(0), status_(Status::Stop), direction_(Direction::A) {}
	static void AddPIOProgram(PIO pio);
	void RunPIOSm(uint idxSm);
	void StartGeneric(uint32_t pattern, int nPulses, Direction direction);
	void StartFullA(int nPulses) { StartGeneric(Pattern_Full_A, nPulses, Direction::A); }
	void StartFullB(int nPulses) { StartGeneric(Pattern_Full_B, nPulses, Direction::B); }
	void StartHalfA(int nPulses) { StartGeneric(Pattern_Half_A, nPulses, Direction::A); }
	void StartHalfB(int nPulses) { StartGeneric(Pattern_Half_B, nPulses, Direction::B); }
	void Stop();
	int GetPosCur() const { return posCur_; }
};

#endif
