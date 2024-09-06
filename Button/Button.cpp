#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

class Button {
private:
	static struct repeating_timer repeatingTimer_;
public:
	static void Initialize(int32_t msecPolling);
private:
	static bool Callback(struct repeating_timer* pRepeatingTimer);
};

struct repeating_timer Button::repeatingTimer_;

void Button::Initialize(int32_t msecPolling)
{
	::add_repeating_timer_ms(msecPolling, Callback, nullptr, &repeatingTimer_);
}

bool Button::Callback(struct repeating_timer* pRepeatingTimer)
{
    printf("Repeat at %lld\n", ::time_us_64());
    return true;
}

int main()
{
	::stdio_init_all();
	Button::Initialize(500);
	for (;;) {
		//printf("%08x\n", num);
	}
}
