//==============================================================================
// Buton.cpp
//==============================================================================
#include <stdio.h>
#include "jxglib/Button.h"

//------------------------------------------------------------------------------
// Button
//------------------------------------------------------------------------------
size_t Button::nButtons_ = 0;
Button* Button::pButtonTbl_[32];
struct repeating_timer Button::repeatingTimer_;
Button::EventBuff Button::eventBuff_;

void Button::Initialize(int32_t msecPolling)
{
	for (size_t i = 0; i < nButtons_; i++) {
		Button* pButton =  pButtonTbl_[i];
		uint gpio = pButton->GetGPIO();
		::gpio_init(gpio);
		::gpio_set_dir(gpio, GPIO_IN);
		::gpio_pull_up(gpio);
	}
	::add_repeating_timer_ms(msecPolling, Callback, nullptr, &repeatingTimer_);
}

Button::Button(const char* name, uint gpio) : name_(name), gpio_(gpio), status_(false)
{
	pButtonTbl_[nButtons_++] = this;
}

bool Button::Callback(struct repeating_timer* pRepeatingTimer)
{
	for (size_t i = 0; i < nButtons_; i++) {
		Button* pButton =  pButtonTbl_[i];
		bool status = !::gpio_get(pButton->gpio_);
		if (eventBuff_.IsFull()) {
			// nothing to do
		} else if (status && !pButton->status_) {
			eventBuff_.WriteData(new Event(EventType::Pressed, pButton));
		} else if (!status && pButton->status_) {
			eventBuff_.WriteData(new Event(EventType::Released, pButton));
		}
		pButton->status_ = status;
	}
    return true;
}

Button::Event* Button::ReadEvent()
{
	return eventBuff_.HasData()? eventBuff_.ReadData() : nullptr;
}
