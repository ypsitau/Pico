//==============================================================================
// StepMotor.cpp
//==============================================================================
#include "jxglib/StepMotor.h"
#include "RotateOut4Bits.pio.h"

//------------------------------------------------------------------------------
// StepMotor
//------------------------------------------------------------------------------
PIO StepMotor::pio_ = nullptr;
int StepMotor::offsetProgram_ = -1;

void StepMotor::AddPIOProgram(PIO pio)
{
	pio_ = pio;
	offsetProgram_ = pio_add_program(pio_, &RotateOut4Bits_program);
}

void StepMotor::RunPIOSm(uint idxSm)
{
	idxSm_ = idxSm;
	RotateOut4Bits_program_init(pio_, idxSm_, offsetProgram_, gpioFirst_);
	pio_sm_set_enabled(pio_, idxSm_, true);
}

void StepMotor::StartGeneric(uint32_t pattern, int nPulses, Direction direction)
{
	nPulsesToSet_ = nPulses;
	status_ = Status::Running;
	direction_ = direction;
	RotateOut4Bits_set_pattern(pio_, idxSm_, pattern, nPulses, nPulsesPerSec_);
}

void StepMotor::Stop()
{
	if (status_ == Status::Stop) return;
	pio_sm_exec(pio_, idxSm_, pio_encode_jmp(offsetProgram_ + RotateOut4Bits_offset_suspend));
	int nPulsesRemain = pio_sm_get_blocking(pio_, idxSm_);
	int nPulsesDone = nPulsesToSet_ - nPulsesRemain;
	posCur_ += (direction_ == Direction::A)? nPulsesDone : -nPulsesDone;
	nPulsesToSet_ = 0;
	status_ = Status::Stop;
}
