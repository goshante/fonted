#pragma warning( disable : 4005 )

#define GLFW_EXPOSE_NATIVE_WIN32
#include "FontTestWindow.h"
#include <GLFW/glfw3native.h>


#include "Font.h"
#include "resource.h"
#include "Utils.h"

static constexpr const int s_vOffset = 50;
static constexpr const int s_hOffset = 50;

void FontTestWindow::_draw()
{
	while (_pw.isActive() && _testingFont)
	{
		_pw.makeCurrent();
		_pw.pollEvents();
		_pw.beginFrame();
		_pw.setBackgroundColor(_backgroundColor);

		for (int y = 0; y < Min(_height, _screen.size()); y++)
		{
			for (int x = 0; x < Min(_width, _screen[y].size()); x++)
			{
				if (_screen[y][x] > 0)
					_pw.setPixel(x, y, _fontColor);
			}
		}
		_pw.endFrame();
	}
	_pw.forceClose();
}

int FontTestWindow::_calcWndHeight(const Font& font)
{
	return s_vOffset * 2 + font.GetHeight() * 4;
}

int FontTestWindow::_calcWndWidth(const Font& font, const std::string& a, const std::string& b, const std::string& c, const std::string& d)
{
	int w = s_hOffset * 2;
	size_t len_a = Max(a.length(), b.length());
	size_t len_b = Max(c.length(), c.length());
	size_t len = Max(len_a, len_b);
	if (len == 0)
		return w;
	return (font.GetWidth() * (int)len) + (font.GetInterval() * (int)(len - 1)) + w;
}

static bool callbackClose(void* owner)
{
	return true;
}

static void callbackMouse(void* owner, pw::mpos pos, int button, int action, int modes)
{
	FontTestWindow* wnd = reinterpret_cast<FontTestWindow*>(owner);
	if (action != 1 || button > 1)
		return;

	if (button == 0)
		wnd->SwitchMonospace();
	else
		wnd->SwitchInvert();
}

void FontTestWindow::SwitchMonospace()
{
	_monospace = !_monospace;
	_screen.Clear();
	_screen.DrawTextRegular(_font, _text, s_hOffset, s_vOffset, 1, _invert, _monospace);
}

void FontTestWindow::SwitchInvert()
{
	_invert = !_invert;
	_screen.Clear();
	_screen.DrawTextRegular(_font, _text, s_hOffset, s_vOffset, 1, _invert, _monospace);
}

FontTestWindow::FontTestWindow(const Font& font, std::atomic<bool>& flag, const std::string& test, const std::string& testCyr, const std::string& nums, const std::string& special, uint32_t background, uint32_t foreground)
	: _font(font)
	, _height(_calcWndHeight(font))
	, _width(_calcWndWidth(font, test, testCyr, nums, special))
	, _pw(_width, _height, "Font test")
	, _backgroundColor(background)
	, _fontColor(foreground)
	, _screen(_width, _height)
	, _monospace(true)
	, _testingFont(flag)
	, _invert(false)
{
	_testingFont = true;
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(glfwGetWin32Window(_pw._getHandle()), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	SendMessage(glfwGetWin32Window(_pw._getHandle()), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	_text = test + "\n" + testCyr + "\n" + nums + "\n" + special;
	_screen.DrawTextRegular(font, _text, s_hOffset, s_vOffset, 1, false, _monospace);
	_pw.setOwner(this);
	_pw.setCloseCallback(&callbackClose);
	_pw.addMouseCallback(&callbackMouse);
	_draw();
}

FontTestWindow::~FontTestWindow()
{
	_pw.forceClose();
	_testingFont = false;
}