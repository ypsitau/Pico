#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "stepping.pio.h"

#define ArraySizeOf(x) (sizeof(x) / sizeof(x[0]))

int main()
{
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &shiftout4bits_program);
	printf("System Frequency: %dHz\n", clock_get_hz(clk_sys));
	do {
		uint freq = 1;
		int idxSm = pio_claim_unused_sm(pio, true);
		uint pinFirst = 12;
		shiftout4bits_program_init(pio, idxSm, offsetProgram, pinFirst);
		pio_sm_set_enabled(pio, idxSm, true);
		pio_sm_put(pio, idxSm, 0b1001'1100'0110'0011'1001'1100'0110'0011);
		//pio_sm_put(pio, idxSm, 0xffffffff);
		pio_sm_put(pio, idxSm, (clock_get_hz(clk_sys) / (2 * freq)) - 3);
	} while (0);
	for (;;) ;
	return 0;
}

#if 0
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
