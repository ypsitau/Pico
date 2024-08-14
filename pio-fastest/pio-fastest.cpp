#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "pio-fastest.pio.h"

int main()
{
	uint32_t freqPeri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI) * 1000;
	stdio_init_all();
	PIOSM_fastest pioSm(pio0, 0, 14);
	//pio_gpio_init
	do {
		pioSm.Init();
		pio_sm_set_enabled(pioSm.GetPIO(), pioSm.GetIdxSm(), true);
	} while (0);
	for (;;) ;
}
