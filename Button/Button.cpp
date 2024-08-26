#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "Button.pio.h"

int main()
{
	stdio_init_all();
	printf("System Frequency: %dHz\n", clock_get_hz(clk_sys));
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &ButtonFrontEnd_program);
	int idxSm = pio_claim_unused_sm(pio, true);
	do {
		uint pinFirst = 16;
		ButtonFrontEnd_program_init(pio, idxSm, offsetProgram, pinFirst, 3);
		pio_sm_set_enabled(pio, idxSm, true);
	} while (0);
	for (;;) {
		uint32_t num = pio_sm_get_blocking(pio, idxSm);
		printf("%08x\n", num);
	}
}
