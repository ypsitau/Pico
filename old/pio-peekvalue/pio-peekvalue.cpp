#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "pio-peekvalue.pio.h"

int main()
{
	stdio_init_all();
	printf("----\n");
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &peekvalue_program);
	int idxSm = pio_claim_unused_sm(pio, true);
	pio_sm_config cfg = peekvalue_program_get_default_config(offsetProgram);
	pio_sm_init(pio, idxSm, offsetProgram, &cfg);
	pio_sm_set_enabled(pio, idxSm, true);
	for (;;) {
		pio_sm_exec(pio, idxSm, pio_encode_push(false, true));
		uint32_t value = pio_sm_get(pio, idxSm);
		printf("%08x\n", value);
		sleep_ms(100);
	}
}
