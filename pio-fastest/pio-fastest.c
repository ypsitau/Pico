#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "pio-fastest.pio.h"

int main()
{
	uint32_t freqPeri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI) * 1000;
	stdio_init_all();
	PIO pio = pio0;
	uint fastest_program_offset = pio_add_program(pio, &fastest_program);
	do {
		uint pin = 14;
		uint idxSm = 0;
		fastest_program_init(pio, idxSm, fastest_program_offset, pin);
		pio_sm_set_enabled(pio, idxSm, true);
	} while (0);
	for (;;) ;
}
