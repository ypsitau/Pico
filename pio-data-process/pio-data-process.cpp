#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "pio-data-process.pio.h"

void StartProgram(const char* label, const pio_program& program, uint32_t numIn, bool shift_right = true)
{
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &program);
	int idxSm = pio_claim_unused_sm(pio, true);
	pio_sm_config cfg = pio_get_default_sm_config();
	sm_config_set_in_shift(&cfg, shift_right, false, 0);
	pio_sm_init(pio, idxSm, offsetProgram, &cfg);
	pio_sm_set_enabled(pio, idxSm, true);
	pio_sm_put(pio, idxSm, numIn);
	uint32_t numOut = pio_sm_get_blocking(pio, idxSm);
	printf("%s\n  0x%08x(0b%032b) -> 0x%08x(0b%032b)\n", label, numIn, numIn, numOut, numOut);
	pio_remove_program(pio, &program, offsetProgram);
	pio_sm_unclaim(pio, idxSm);
}

int main()
{
	stdio_init_all();
	printf("----\n");
	StartProgram("echo back", test1_program, 0x12345678);
	StartProgram("{mov isr, ::osr}", test2_program, 0x89abcdef);
	StartProgram("{mov isr, !osr}", test3_program, 0x89abcdef);
	StartProgram("{in null, 6} (shift_right = true)", test4_program, 0x89abcdef, true);
	StartProgram("{in null, 6} (shift_right = false)", test4_program, 0x89abcdef, false);
	StartProgram("{set x, 0b10111; in x, 6} (shift_right = true)", test5_program, 0x89abcdef, true);
	StartProgram("{set x, 0b10111; in x, 6} (shift_right = false)", test5_program, 0x89abcdef, false);
	StartProgram("{in isr, 6} (shift_right = true)", test6_program, 0x89abcdea, true);
	StartProgram("{in isr, 6} (shift_right = false)", test6_program, 0x89abcdea, false);
	for (;;) tight_loop_contents();
}
