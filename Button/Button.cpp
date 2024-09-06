#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include "FIFOBuff.h"

class Button {
public:
	enum class EventType { None, Press, Release };
	class Event {
	private:
		EventType eventType_;
		const Button* pButton_;
	public:
		Event() : eventType_(EventType::None), pButton_(nullptr) {}
		Event(const Event& event) : eventType_(event.eventType_), pButton_(event.pButton_) {}
		Event(EventType eventType, const Button* pButton) : eventType_(eventType), pButton_(pButton) {}
	};
	using EventFIFO = FIFOBuff<Event, 32>;
private:
	uint gpio_;
	volatile bool status_;
private:
	static size_t nButtons_;
	static Button* pButtonTbl_[32];
	static struct repeating_timer repeatingTimer_;
	static EventFIFO eventFIFO_;
public:
	static void Initialize(int32_t msecPolling);
public:
	Button(uint gpio);
	uint GetGPIO() const { return gpio_; }
	bool GetStatus() const { return status_; }
private:
	static bool Callback(struct repeating_timer* pRepeatingTimer);
};

size_t Button::nButtons_ = 0;
Button* Button::pButtonTbl_[32];
struct repeating_timer Button::repeatingTimer_;
Button::EventFIFO Button::eventFIFO_;

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

Button::Button(uint gpio) : gpio_(gpio), status_(false)
{
	pButtonTbl_[nButtons_++] = this;
}

bool Button::Callback(struct repeating_timer* pRepeatingTimer)
{
	for (size_t i = 0; i < nButtons_; i++) {
		Button& button =  *pButtonTbl_[i];
		bool status = !::gpio_get(button.gpio_);
		if (status && !button.status_) {

		} else if (!status && button.status_) {

		}
		button.status_ = status;
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
			buttonLeft.GetStatus(), buttonUp.GetStatus(), buttonDown.GetStatus(), buttonRight.GetStatus(),
			buttonA.GetStatus(), buttonB.GetStatus());
		sleep_ms(100);
	}
}
