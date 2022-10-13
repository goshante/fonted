#pragma once
#include "Font.h"

class VirtualCanvas
{
public:
	struct Dims
	{
		int a_x;
		int a_y;
		int b_x;
		int b_y;
	};

private:
	bitmap_t	_canvas;
	int			_width;
	int			_height;

	VirtualCanvas(VirtualCanvas&) = delete;
	VirtualCanvas& operator=(VirtualCanvas&) = delete;

public:
	VirtualCanvas(int w, int h);
	~VirtualCanvas();

	bitmap_t GetBitmap() const;

	Dims DrawTextRegular(const Font& font, const std::string& text, int off_x, int off_y, int brush = 1, bool invert = false);
	Dims DrawRect(int a_x, int a_y, int b_x, int b_y, int brush);
	void Clear();
	void ReInit(int w, int h);
};