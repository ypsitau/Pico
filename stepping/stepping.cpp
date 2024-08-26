#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "stepping.pio.h"

class StepMotor {
public:
	enum class Status { Stop, Running };
	enum class Direction { A, B };
private:
	uint idxSm_;
	int gpioFirst_;
	int posCur_;
	int nPulsesToSet_;
	Status status_;
	Direction direction_;
private:
	static PIO pio_;
	static int offsetProgram_;
public:
	const int nPulsesPerSec = 500;
	const uint32_t Pattern_Full_A = 0b1100'0110'0011'1001'1100'0110'0011'1001;
	const uint32_t Pattern_Full_B = 0b1100'1001'0011'0110'1100'1001'0011'0110;
	const uint32_t Pattern_Half_A = 0b1100'1000'1001'0001'0011'0010'0110'0100;
	const uint32_t Pattern_Half_B = 0b1100'0100'0110'0010'0011'0001'1001'1000;
public:
	StepMotor(uint idxSm, int gpioFirst) : idxSm_(idxSm), gpioFirst_(gpioFirst),
				posCur_(0), nPulsesToSet_(0), status_(Status::Stop), direction_(Direction::A) {}
	static void Initialize(PIO pio);
	void Enable();
	void Stop();
	void StartGeneric(uint32_t pattern, int nPulses, Direction direction);
	void StartFullA(int nPulses) { StartGeneric(Pattern_Full_A, nPulses, Direction::A); }
	void StartFullB(int nPulses) { StartGeneric(Pattern_Full_B, nPulses, Direction::B); }
	void StartHalfA(int nPulses) { StartGeneric(Pattern_Half_A, nPulses, Direction::A); }
	void StartHalfB(int nPulses) { StartGeneric(Pattern_Half_B, nPulses, Direction::B); }
	int GetPosCur() const { return posCur_; }
};

PIO StepMotor::pio_ = nullptr;
int StepMotor::offsetProgram_ = -1;

void StepMotor::Initialize(PIO pio)
{
	pio_ = pio;
	offsetProgram_ = pio_add_program(pio_, &rotateout4bits_program);
}

void StepMotor::Enable()
{
	rotateout4bits_program_init(pio_, idxSm_, offsetProgram_, gpioFirst_);
	pio_sm_set_enabled(pio_, idxSm_, true);
}

void StepMotor::Stop()
{
	if (status_ == Status::Stop) return;
	pio_sm_exec(pio_, idxSm_, pio_encode_jmp(offsetProgram_ + rotateout4bits_offset_suspend));
	int nPulsesRemain = pio_sm_get_blocking(pio_, idxSm_);
	int nPulsesDone = nPulsesToSet_ - nPulsesRemain;
	posCur_ += (direction_ == Direction::A)? nPulsesDone : -nPulsesDone;
	nPulsesToSet_ = 0;
	status_ = Status::Stop;
}

void StepMotor::StartGeneric(uint32_t pattern, int nPulses, Direction direction)
{
	nPulsesToSet_ = nPulses;
	status_ = Status::Running;
	direction_ = direction;
	rotateout4bits_set_pattern(pio_, idxSm_, pattern, nPulses, nPulsesPerSec);
}

int main()
{
	stdio_init_all();
	StepMotor::Initialize(pio0);
	StepMotor stepMotor(0, 12);
	stepMotor.Enable();
	printf("System Frequency: %dHz (measured: %d000Hz)\n",
			clock_get_hz(clk_sys), frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS));
	for (int i = 0; i < 3; i++) {
		uint gpio = 16 + i;
		gpio_init(gpio);
		gpio_set_dir(gpio, GPIO_IN);
		gpio_pull_up(gpio);
	}
	for (;;) {
		if (!gpio_get(16)) {
			stepMotor.Stop();
			printf("current position: %d\n", stepMotor.GetPosCur());
			stepMotor.StartFullA(-1);
			sleep_ms(500);
		}
		if (!gpio_get(17)) {
			stepMotor.Stop();
			printf("current position: %d\n", stepMotor.GetPosCur());
			stepMotor.StartFullB(-1);
			sleep_ms(500);
		}
		if (!gpio_get(18)) {
			stepMotor.Stop();
			printf("current position: %d\n", stepMotor.GetPosCur());
			sleep_ms(500);
		}
	}
	return 0;
}

#if 0
#define ArraySizeOf(x) (sizeof(x) / sizeof(x[0]))

struct ControlPin {
	bool pinA, pinB, pinC, pinD;
};

int main()
{
	stdio_init_all();
	gpio_init(12);
	gpio_init(13);
	gpio_init(14);
	gpio_init(15);
	gpio_set_dir(12, GPIO_OUT);
	gpio_set_dir(13, GPIO_OUT);
	gpio_set_dir(14, GPIO_OUT);
	gpio_set_dir(15, GPIO_OUT);
	int delayUSec = 2000;
#if 0
	// Wave drive
	static const ControlPin controlPinTbl[] = {
		{ true, false, false, false },
		{ false, true, false, false },
		{ false, false, true, false },
		{ false, false, false, true },
	};
#endif
#if 1
	// Full step
	static const ControlPin controlPinTbl[] = {
		{ true, true, false, false },
		{ false, true, true, false },
		{ false, false, true, true },
		{ true, false, false, true },
	};
#endif
#if 0
	// Half step
	static const ControlPin controlPinTbl[] = {
		{ true, true, false, false },
		{ false, true, false, false },
		{ false, true, true, false },
		{ false, false, true, false },
		{ false, false, true, true },
		{ false, false, false, true },
		{ true, false, false, true },
		{ true, false, false, false },
	};
#endif
	int iTbl = 0;
	int nStepsFirst = 100;
	//for (int i = 0; i < 2048; i++) {
	for (int i = 0; ; i++) {
		ControlPin controlPin = controlPinTbl[iTbl];	
		gpio_put(12, controlPin.pinA);
		gpio_put(13, controlPin.pinB);
		gpio_put(14, controlPin.pinC);
		gpio_put(15, controlPin.pinD);
		//sleep_us((i < nStepsFirst)? 2500 : 1600);
		sleep_us(2000);
		if (++iTbl >= ArraySizeOf(controlPinTbl)) iTbl = 0;
	}
}
#endif
