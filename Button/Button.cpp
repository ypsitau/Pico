#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

class Button {
private:
	uint gpio_;
	volatile bool pushedFlag_;
private:
	static size_t nButtons_;
	static Button* pButtonTbl_[32];
	static struct repeating_timer repeatingTimer_;
public:
	static void Initialize(int32_t msecPolling);
public:
	Button(uint gpio);
	uint GetGPIO() const { return gpio_; }
	bool IsPushed() const { return pushedFlag_; }
private:
	static bool Callback(struct repeating_timer* pRepeatingTimer);
};

size_t Button::nButtons_ = 0;
Button* Button::pButtonTbl_[32];
struct repeating_timer Button::repeatingTimer_;

void Button::Initialize(int32_t msecPolling)
{
	for (size_t i = 0; i < nButtons_; i++) {
		Button& button =  *pButtonTbl_[i];
		uint gpio = button.GetGPIO();
		::gpio_init(gpio);
		::gpio_set_dir(gpio, GPIO_IN);
		::gpio_pull_up(gpio);
	}
	::add_repeating_timer_ms(msecPolling, Callback, nullptr, &repeatingTimer_);
}

Button::Button(uint gpio) : gpio_(gpio), pushedFlag_(false)
{
	pButtonTbl_[nButtons_++] = this;
}

bool Button::Callback(struct repeating_timer* pRepeatingTimer)
{
	for (size_t i = 0; i < nButtons_; i++) {
		Button& button =  *pButtonTbl_[i];
		button.pushedFlag_ = !::gpio_get(button.gpio_);
	}
    return true;
}


Button buttonLeft(10), buttonUp(11), buttonDown(12), buttonRight(13);
Button buttonA(14), buttonB(15);

int main()
{
	::stdio_init_all();
	Button::Initialize(10);
	for (;;) {
		printf("%d %d %d %d %d %d\n",
			buttonLeft.IsPushed(), buttonUp.IsPushed(), buttonDown.IsPushed(), buttonRight.IsPushed(),
			buttonA.IsPushed(), buttonB.IsPushed());
		sleep_ms(100);
	}
}
