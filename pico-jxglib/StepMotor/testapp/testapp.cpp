#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <jxglib/StepMotor.h>

int main()
{
	stdio_init_all();
	printf("System Frequency: %dHz\n", clock_get_hz(clk_sys));
	StepMotor::AddPIOProgram(pio0);
	StepMotor stepMotorL(6, 400);
	StepMotor stepMotorR(10, 400);
	stepMotorL.RunPIOSm(::pio_claim_unused_sm(pio0, true));
	stepMotorR.RunPIOSm(::pio_claim_unused_sm(pio0, true));
	::printf("Left Right\n");
	::printf("[Q]  [W]    Forward\n");
	::printf("[A]  [S]    Stop\n");
	::printf("[Z]  [X]    Backward\n");
	for (;;) {
		int ch = ::fgetc(stdin);
		if (ch == 'q') {
			stepMotorL.Stop();
			::printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorL.StartFullA(-1);
		}
		if (ch == 'z') {
			stepMotorL.Stop();
			::printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorL.StartFullB(-1);
		}
		if (ch == 'w') {
			stepMotorR.Stop();
			::printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorR.StartFullA(-1);
		}
		if (ch == 'x') {
			stepMotorR.Stop();
			::printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
			stepMotorR.StartFullB(-1);
		}
		if (ch == 'a') {
			stepMotorL.Stop();
			::printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
		}
		if (ch == 's') {
			stepMotorR.Stop();
			::printf("current position: %d, %d\n", stepMotorL.GetPosCur(), stepMotorR.GetPosCur());
		}
	}
	return 0;
}
