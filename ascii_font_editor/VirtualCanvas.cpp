#pragma once
#include "VirtualCanvas.h"
#include "Utils.h"
#include <cmath>

#define mod(n) (n < 0 ? n * -1 : n)

VirtualCanvas::VirtualCanvas(int w, int h)
	: _width(w)
	, _height(h)
{
	InitBitmap(_canvas, h, w);
}

VirtualCanvas::~VirtualCanvas()
{
}

bitmap_t VirtualCanvas::GetBitmap() const
{
	return _canvas;
}

VirtualCanvas::Dims VirtualCanvas::DrawRect(int a_x, int a_y, int b_x, int b_y, int brush)
{
	for (int x = 0; x < b_x - a_x; x++)
	{
		for (int y = 0; y < b_y - a_y; y++)
			_canvas[a_y + y][a_x + x] = brush;
	}

	return { a_x, a_y, b_x, b_y };
}

VirtualCanvas::Dims VirtualCanvas::DrawTextRegular(const Font& font, const std::string& text, int off_x, int off_y, int brush, bool invert, bool monospace)
{
	int fh = font.GetHeight();
	Dims dims;
	dims.a_x = off_x;
	dims.a_y = off_y;
	dims.b_x = off_x;
	dims.b_y = off_y + fh;
	int initialX = off_x;

	int cur_row_w = off_x;
	for (size_t i = 0; i < text.length(); i++)
	{
		char c = text[i];
		auto letter = font.GetCharImage_8bit((unsigned char)c, monospace);

		if (letter.empty())
			continue;

		int fw = (int)letter[0].size();

		if (c == '\r')
			continue;

		if (c == '\n')
		{
			off_y += 1 + fh;
			dims.b_y += 1 + fh;
			

			if (cur_row_w > dims.b_x)
				dims.b_x = cur_row_w;

			off_x = initialX;
			cur_row_w = initialX;
			continue;
		}

		for (int y = 0; y < fh; y++)
		{
			for (int x = 0; x < fw; x++)
			{
				int pos_x = x + off_x;
				int pos_y = y + off_y;
				if (pos_x >= _width || pos_y >= _height || pos_x < 0 || pos_y < 0)
					break;

				if ((!invert && letter[y][x] == 1) ||
					(invert && letter[y][x] == 0))
					_canvas[pos_y][pos_x] = brush;
			}
		}
		off_x += fw + font.GetInterval();
		cur_row_w += fw + font.GetInterval();
	}

	if (dims.b_x == initialX)
		dims.b_x = cur_row_w;
	return dims;
}

void VirtualCanvas::Clear()
{
	InitBitmap(_canvas, _height, _width);
}

void VirtualCanvas::ReInit(int w, int h)
{
	_height = h;
	_width = w;
	InitBitmap(_canvas, _height, _width);
}

const std::vector<pixel_t>& VirtualCanvas::operator[](size_t i) const
{
	return _canvas[i];
}

const size_t VirtualCanvas::size() const
{
	return _canvas.size();
}