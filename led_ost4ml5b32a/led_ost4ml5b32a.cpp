#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "led_ost4ml5b32a.pio.h"

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

int main()
{
	stdio_init_all();
	StartProgram(shift_program, 0x01234567);
	for (;;) tight_loop_contents();
}
