#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "blink.pio.h"

int main()
{
	uint32_t freqPeri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI) * 1000;
	stdio_init_all();
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &blink_program);
	do {
		uint pin = 6;
		uint idxSm = 0;
		uint freq = 3;
		blink_program_init(pio, idxSm, offsetProgram, pin);
		pio_sm_set_enabled(pio, idxSm, true);
		pio->txf[idxSm] = (freqPeri / (2 * freq)) - 3;
	} while (0);
	do {
		uint pin = 7;
		uint idxSm = 1;
		uint freq = 4;
		blink_program_init(pio, idxSm, offsetProgram, pin);
		pio_sm_set_enabled(pio, idxSm, true);
		pio->txf[idxSm] = (freqPeri / (2 * freq)) - 3;
	} while (0);
	do {
		uint pin = 8;
		uint idxSm = 2;
		uint freq = 1;
		blink_program_init(pio, idxSm, offsetProgram, pin);
		pio_sm_set_enabled(pio, idxSm, true);
		pio->txf[idxSm] = (freqPeri / (2 * freq)) - 3;
	} while (0);
	while (true) {
		printf("Hello, world!\n");
		sleep_ms(1000);
	}
}
