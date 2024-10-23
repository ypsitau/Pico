#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "jxglib/StepMotor.h"

int main()
{
	stdio_init_all();
	printf("System Frequency: %dHz\n", clock_get_hz(clk_sys));
	//printf("System Frequency: %dHz (measured: %d000Hz)\n",
	//		clock_get_hz(clk_sys), frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS));
	StepMotor::Initialize(pio0);
	StepMotor stepMotorL(0, 8, 400);
	StepMotor stepMotorR(1, 12, 400);
	stepMotorL.Enable();
	stepMotorR.Enable();
	for (int i = 0; i < 6; i++) {
		uint gpio = 16 + i;
		gpio_init(gpio);
		gpio_set_dir(gpio, GPIO_IN);
		gpio_pull_up(gpio);
	}
	for (;;) {
		if (!gpio_get(16)) {
			stepMotorL.Stop();
			printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorL.StartFullA(-1);
			sleep_ms(500);
		}
		if (!gpio_get(17)) {
			stepMotorL.Stop();
			printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorL.StartFullB(-1);
			sleep_ms(500);
		}
		if (!gpio_get(18)) {
			stepMotorR.Stop();
			printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorR.StartFullA(-1);
			sleep_ms(500);
		}
		if (!gpio_get(19)) {
			stepMotorR.Stop();
			printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorR.StartFullB(-1);
			sleep_ms(500);
		}
		if (!gpio_get(20)) {
			stepMotorL.Stop();
			stepMotorR.Stop();
			printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			sleep_ms(500);
		}
	}
	return 0;
}
