#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "pio-fastest.pio.h"

int main()
{
	stdio_init_all();
	PIO pio = pio0;
	printf("System Frequency: %dHz\n", clock_get_hz(clk_sys));
	uint offsetProgram = pio_add_program(pio, &fastest_program);
	do {
		int idxSm = pio_claim_unused_sm(pio, true);
		uint pinFirst = 14;
		fastest_program_init(pio, idxSm, offsetProgram, pinFirst);
		pio_sm_set_enabled(pio, idxSm, true);
	} while (0);
	for (;;) tight_loop_contents();
}
