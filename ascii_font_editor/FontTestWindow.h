#pragma once
#include <PixelWindow/PixelWindow.h>
#include "VirtualCanvas.h"

class Font;

class FontTestWindow
{
private:
	const Font& _font;
	int _height;
	int _width;
	pw::PixelWindow _pw;
	uint32_t _backgroundColor;
	uint32_t _fontColor;
	std::string _text;
	VirtualCanvas _screen;
	bool _monospace;

	FontTestWindow(FontTestWindow&) = delete;
	FontTestWindow& operator=(FontTestWindow&) = delete;

	void _draw();
	static int _calcWndHeight(const Font& font);
	static int _calcWndWidth(const Font& font, const std::string& a, const std::string& b, const std::string& c, const std::string& d);

public:
	FontTestWindow(const Font& font, const std::string& test, const std::string& testCyr, const std::string& nums, const std::string& special, uint32_t background = 0x00000000, uint32_t foreground = 0xFFFFFFFF);
	~FontTestWindow();
	void SwitchMonospace();
};