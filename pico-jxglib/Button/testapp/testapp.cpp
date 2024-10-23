#include <stdio.h>
#include <hardware/clocks.h>
#include "jxglib/Button.h"

Button buttonLeft("Left", 10), buttonUp("Up", 11), buttonDown("Down", 12), buttonRight("Right", 13);
Button buttonA("A", 14), buttonB("B", 15);

int main()
{
	::stdio_init_all();
	Button::Initialize(50);
	int n = 0;
	for (;;) {
		for (;;) {
			std::unique_ptr<Button::Event> pEvent(Button::ReadEvent());
			if (!pEvent) break;
			//printf("%-8s %s\n", pEvent->IsPressed()? "Pressed" : "Released", pEvent->GetName());
			if (pEvent->IsPressed()) printf("%s\n", pEvent->GetName());
			//if (pEvent->IsPressed(buttonLeft)) {
			//	n++;
			//	printf("count: %d\n", n);
			//}
		}
		sleep_ms(100);
	}
}
