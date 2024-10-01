#include <stdio.h>
#include "SSD1306.h"

void SSD1306::PutPixel(int x, int y, bool on)
{
	uint8_t* buf = raw.GetBitPattern();
	int byte_idx = (y / 8) * Width + x;
	uint8_t byte = buf[byte_idx];

	if (on)
		byte |=  1 << (y % 8);
	else
		byte &= ~(1 << (y % 8));

	buf[byte_idx] = byte;
}

void SSD1306::DrawLine(int x0, int y0, int x1, int y1, bool on)
{
	int dx =  abs(x1-x0);
	int sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1-y0);
	int sy = y0<y1 ? 1 : -1;
	int err = dx+dy;
	int e2;

	while (true) {
		PutPixel(x0, y0, on);
		if (x0 == x1 && y0 == y1)
			break;
		e2 = 2*err;

		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}
