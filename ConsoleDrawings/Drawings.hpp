#pragma once
#include <Windows.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include "Configs.hpp"


#define PI 3.14


HDC hdc;
COLORREF* pane = (COLORREF*)calloc(Cw * Ch, sizeof(COLORREF));


inline void SETPIX(int x, int y, COLORREF color)
{
	if (y * Cw + x > Cw * Ch || y * Cw + x < 0) return;
	pane[y * Cw + x] = color;
}


inline void draw()
{
	HBITMAP map = CreateBitmap(Cw, Ch,
		1, // Color Planes, unfortanutelly don't know what is it actually. Let it be 1
		8 * 4, // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
		(void*)pane); // pointer to array
	HDC src = CreateCompatibleDC(hdc);
	SelectObject(src, map); // Inserting picture into our temp HDC
	BitBlt(hdc, 0, 0, Cw, Ch, src,
		0, 0,   // x and y of upper left corner  of part of the source, from where we'd like to copy
		SRCCOPY); // Defined DWORD to juct copy pixels. Watch more on msdn;
	DeleteObject(map);
	DeleteDC(src);
}


inline void Clear(COLORREF color)
{
	for (int x = 0; x < Cw; ++x)
		for (int y = 0; y < Ch; ++y)
			SETPIX(x, y, color);
}


inline void HorizontalLine(int x0, int x1, int y, COLORREF color)
{
	for (int x = x0; x < x1; ++x)
		SETPIX(x, y, color);
}



inline void VerticalLine(int y0, int y1, int x, COLORREF color)
{
	for (int y = y0; y < y1; ++y)
		SETPIX(x, y, color);
}



inline void DrawLine(int x1, int y1, int x2, int y2, COLORREF color)
{
	double x = x2 - x1;
	double y = y2 - y1;
	double length = sqrt(x * x + y * y);

	double addx = x / length;
	double addy = y / length;

	x = x1;
	y = y1;

	for (double i = 0; i < length; i += 1)
	{
		SETPIX(x, y, color);
		x += addx;
		y += addy;
	}

}



// O MEU PROGRAMA DESENHA NO SENTIDO HORARIO
inline void Rectangle(int x, int y, int width, int height, COLORREF color)
{
	HorizontalLine(x, x + width, y, color);
	VerticalLine(y, y + height, x + width, color);
	HorizontalLine(x, x + width, y + height, color);
	VerticalLine(y, y + height, x, color);
}



inline void FilledRect(int x, int y, int width, int height, COLORREF color)
{
	for (int _x = x; _x < x + width; ++_x)
		for (int _y = y; _y < y + height; ++_y)
			SETPIX(x, y, color);
}



inline void Triangle(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color)
{
	DrawLine(x1, y1, x2, y2, color);
	DrawLine(x2, y2, x3, y3, color);
	DrawLine(x3, y3, x1, y1, color);
}



inline void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color)
{
	auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; ++i) SETPIX(i, ny, color); };

	int t1x, t2x, y, minx, maxx, t1xp, t2xp;
	bool changed1 = false;
	bool changed2 = false;
	int signx1, signx2, dx1, dy1, dx2, dy2;
	int e1, e2;
	// Sort vertices
	if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
	if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3); }
	if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3); }

	t1x = t2x = x1; y = y1;   // Starting points
	dx1 = (int)(x2 - x1);
	if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (int)(y2 - y1);

	dx2 = (int)(x3 - x1);
	if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
	else signx2 = 1;
	dy2 = (int)(y3 - y1);

	if (dy1 > dx1) { std::swap(dx1, dy1); changed1 = true; }
	if (dy2 > dx2) { std::swap(dy2, dx2); changed2 = true; }

	e2 = (int)(dx2 >> 1);
	// Flat top, just process the second half
	if (y1 == y2) goto next;
	e1 = (int)(dx1 >> 1);

	for (int i = 0; i < dx1;) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			i++;
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) t1xp = signx1;//t1x += signx1;
				else          goto next1;
			}
			if (changed1) break;
			else t1x += signx1;
		}
		// Move line
	next1:
		// process second line until y value is about to change
		while (1) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;//t2x += signx2;
				else          goto next2;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next2:
		if (minx > t1x) minx = t1x;
		if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x;
		if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);    // Draw line from min to max points found on the y
									// Now increase y
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y == y2) break;
	}
next:
	// Second half
	dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (int)(y3 - y2);
	t1x = x2;

	if (dy1 > dx1) {   // swap values
		std::swap(dy1, dx1);
		changed1 = true;
	}
	else changed1 = false;

	e1 = (int)(dx1 >> 1);

	for (int i = 0; i <= dx1; ++i) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) { t1xp = signx1; break; }//t1x += signx1;
				else          goto next3;
			}
			if (changed1) break;
			else   	   	  t1x += signx1;
			if (i < dx1) i++;
		}
	next3:
		// process second line until y value is about to change
		while (t2x != x3) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;
				else          goto next4;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next4:

		if (minx > t1x) minx = t1x;
		if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x;
		if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y > y3) return;
	}
}