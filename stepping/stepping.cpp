#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "stepping.pio.h"

#define ArraySizeOf(x) (sizeof(x) / sizeof(x[0]))

void StartProgram(const pio_program& program, uint32_t numIn, bool shift_right = true)
{
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &program);
	int idxSm = pio_claim_unused_sm(pio, true);
	pio_sm_config cfg = shift_program_get_default_config(offsetProgram);
	sm_config_set_in_shift(&cfg, shift_right, false, 0);
	pio_sm_init(pio, idxSm, offsetProgram, &cfg);
	pio_sm_set_enabled(pio, idxSm, true);
	pio_sm_put(pio, idxSm, numIn);
	for (int i = 0; i < 32; i++) {
		uint32_t numOut = pio_sm_get_blocking(pio, idxSm);
		printf("0b%032b\n", numOut);
		sleep_ms(100);
	}
	pio_remove_program(pio, &program, offsetProgram);
	pio_sm_unclaim(pio, idxSm);
}

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
	for (int i = 0; i < 2048; i++) {
		ControlPin controlPin = controlPinTbl[iTbl];	
		gpio_put(12, controlPin.pinA);
		gpio_put(13, controlPin.pinB);
		gpio_put(14, controlPin.pinC);
		gpio_put(15, controlPin.pinD);
		sleep_us((i < nStepsFirst)? 2500 : 1600);
		if (++iTbl >= ArraySizeOf(controlPinTbl)) iTbl = 0;
	}
}
