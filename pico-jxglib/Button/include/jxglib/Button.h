//==============================================================================
// jxglib/Buton.h
//==============================================================================
#ifndef PICO_JXGLIB_BUTTON_H
#define PICO_JXGLIB_BUTTON_H
#include <stdio.h>
#include <memory>
#include "pico/stdlib.h"
#include "jxglib/FIFOBuff.h"

//------------------------------------------------------------------------------
// Button
//------------------------------------------------------------------------------
class Button {
public:
	enum class EventType { None, Pressed, Released };
	class Event {
	private:
		EventType eventType_;
		const Button* pButton_;
	public:
		Event() : eventType_(EventType::None), pButton_(nullptr) {}
		Event(EventType eventType, const Button* pButton) : eventType_(eventType), pButton_(pButton) {}
		bool IsPressed() const { return eventType_ == EventType::Pressed; }
		bool IsReleased() const { return eventType_ == EventType::Released; }
		bool IsPressed(const Button& button) const { return IsPressed() && pButton_ == &button; }
		bool IsReleased(const Button& button) const { return IsReleased() && pButton_ == &button; }
		const char* GetName() const { return pButton_->GetName(); }
	};
	using EventBuff = FIFOBuff<Event*, 64>;
private:
	const char* name_;
	uint gpio_;
	volatile bool status_;
private:
	static size_t nButtons_;
	static Button* pButtonTbl_[32];
	static struct repeating_timer repeatingTimer_;
	static EventBuff eventBuff_;
public:
	static void Initialize(int32_t msecPolling);
	static Event* ReadEvent();
public:
	Button(const char* name, uint gpio);
	uint GetGPIO() const { return gpio_; }
	const char* GetName() const { return name_; }
	bool GetStatus() const { return status_; }
private:
	static bool Callback(struct repeating_timer* pRepeatingTimer);
};

#endif
