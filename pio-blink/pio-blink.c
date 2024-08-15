#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "blink.pio.h"

int main()
{
	stdio_init_all();
	PIO pio = pio0;
	uint offsetProgram = pio_add_program(pio, &blink_program);
	do {
		uint pin = 10;
		uint idxSm = 0;
		uint freq = 3;
		blink_program_init(pio, idxSm, offsetProgram, pin);
		blink_start(pio, idxSm, freq);
	} while (0);
	do {
		uint pin = 11;
		uint idxSm = 1;
		uint freq = 4;
		blink_program_init(pio, idxSm, offsetProgram, pin);
		blink_start(pio, idxSm, freq);
	} while (0);
	do {
		uint pin = 12;
		uint idxSm = 2;
		uint freq = 1;
		blink_program_init(pio, idxSm, offsetProgram, pin);
		blink_start(pio, idxSm, freq);
	} while (0);
	while (true) {
		printf("Hello, world!\n");
		sleep_ms(1000);
	}
}
