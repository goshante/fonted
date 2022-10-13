#include "Canvas.h"
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "MenuFont.h"

#define Min(a,b) (a < b ? a : b)

constexpr const int MenuHeight = 16;

using namespace std::chrono_literals;

Canvas::Canvas(int w, int h, int scale, const std::string& title, bool startHidden, uint32_t brush, uint32_t background)
	: _pw(nullptr)
	, _title(title)
	, _width(w)
	, _height(h)
	, _background(background)
	, _brush(brush)
	, _closed(startHidden)
	, _pointX(0)
	, _pointY(0)
	, _scale(scale)
	, _doDraw(0)
	, _cc(false)
	, _mc(false)
	, _clc(false)
	, _useMarker(true)
	, _menuFont(getMenuFont(), 14, 8)
	, _menuFrame(w, MenuHeight)
	, _lastButton(MenuButtons::None)
	, _lastMenuClick(std::chrono::system_clock::now())
	, _owner(nullptr)
	, _holdDraw(false)
	, _lastDrawCall(std::chrono::system_clock::now())
{
	InitBitmap(_frame, _height, _width);
	if (!_closed)
		_thread = std::make_shared<std::thread>(&Canvas::_draw, this);
}

Canvas::~Canvas()
{
	Close();
}

bitmap_t Canvas::GetPicture() const
{
	std::lock_guard<std::mutex> guard(_lock);
	return _frame;
}

void Canvas::Show(const bitmap_t& bmp)
{
	if (!_closed)
		return;

	if (!bmp.empty())
		_frame = bmp;
	else
		InitBitmap(_frame, _height, _width);
	_closed = false;
	_thread = std::make_shared<std::thread>(&Canvas::_draw, this);
}

void Canvas::Close()
{
	if (_closed)
		return;

	_closed = true;
	_thread->join();
}

void Canvas::SetPicture(const bitmap_t& picture)
{
	std::lock_guard<std::mutex> guard(_lock);
	_frame = picture;
}

void Canvas::MovePoint(int x, int y)
{
	std::lock_guard<std::mutex> guard(_lock);

	if (_lastButton != MenuButtons::None)
	{
		_lastButton = MenuButtons::None;
		_redrawMenu();
	}

	_pointX = _pointX - x;
	_pointX = _pointX < 0 ? 0 : _pointX;
	_pointX = _pointX >= _width ? _width - 1 : _pointX;

	_pointY = _pointY - y;
	_pointY = _pointY < 0 ? 0 : _pointY;
	_pointY = _pointY >= _height ? _height - 1 : _pointY;
}

bool Canvas::SetPoint(int x, int y)
{
	std::lock_guard<std::mutex> guard(_lock);

	if (_lastButton != MenuButtons::None)
	{
		_lastButton = MenuButtons::None;
		_redrawMenu();
	}

	if (x >= _width)
		return false;

	if (x < 0)
		return false;

	if (y - MenuHeight >= _height)
		return false;

	if (y - MenuHeight < 0)
		return false;


	_pointX = x;
	_pointY = y - MenuHeight;

	return true;
}

void Canvas::Draw(bool erase, bool hold)
{
	Canvas::TimePoint now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto last_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(_lastDrawCall);
	bool singleDraw = false;
	if (now_ms - last_ms < 333ms && !hold)
		singleDraw = true;
	_lastDrawCall = now;

	if (_holdDraw && !hold)
	{
		_holdDraw = false;
		_doDraw = 0;

		if (!singleDraw)
			return;
	}

	_doDraw = erase ? 2 : 1;
	_holdDraw = hold;
}

void callbackMouse(void* owner, pw::mpos pos, int button, int action, int modes)
{
	Canvas* canv = reinterpret_cast<Canvas*>(owner);
	try
	{
		bool menu = false;
		if (pos.y < MenuHeight * canv->_scale)
			menu = true;
		pos.x = pos.x / canv->_scale;
		pos.y = pos.y / canv->_scale;

		Canvas::MenuButtons btn = Canvas::MenuButtons::None;
		if (menu && canv->_mc && button == 0)
		{
			canv->_doDraw = 0;
			canv->_holdDraw = false;
			if (action == 0)
				return;

			Canvas::TimePoint now = std::chrono::system_clock::now();
			auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

			for (size_t i = 0; i < canv->_menuButtons.size(); i++)
			{
				if (pos.x >= canv->_menuButtons[i].a_x &&
					pos.x <= canv->_menuButtons[i].b_x &&
					pos.y >= canv->_menuButtons[i].a_y &&
					pos.y <= canv->_menuButtons[i].b_y)
				{
					btn = Canvas::MenuButtons(i);
					break;
				}
			}

			if (btn == Canvas::MenuButtons::MarkerSwitch)
			{
				canv->_lastMenuClick = now;
				canv->_useMarker = !canv->_useMarker;
				canv->_redrawMenu();
				return;
			}

			if (btn != Canvas::MenuButtons::None && btn != Canvas::MenuButtons::MarkerSwitch)
			{
				canv->_lastMenuClick = now;
				canv->_lastButton = btn;
				canv->_redrawMenu();
				canv->_callbackMenu(*canv, btn);
			}
		}
		else if (!menu && canv->_cc)
			canv->_callbackCanvas(*canv, pos, button, action, modes);
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_OK | MB_ICONERROR);
	}
}

bool callbackClose(void* owner)
{
	Canvas* canv = reinterpret_cast<Canvas*>(owner);
	if (canv->_clc)
	{
		if (canv->_reinit)
			canv->_closed = true;
		return canv->_callbackClose(*canv);
	}

	return false;
}

void callbackCursor(void* owner, pw::mpos pos)
{
	Canvas* canv = reinterpret_cast<Canvas*>(owner);
	if (pos.y < MenuHeight * canv->_scale || pos.x > canv->_width * canv->_scale || pos.y - (MenuHeight * canv->_scale) > canv->_height * canv->_scale)
	{
		canv->_pw->hideCursor(false);
		return;
	}
	
	if (canv->_useMarker)
		canv->_pw->hideCursor(true);
	canv->SetPoint(int(pos.x / canv->_scale), int(pos.y / canv->_scale));
}

void Canvas::_redrawMenu()
{
	static bool firstDraw = false;
	_menuFrame.Clear();
	_menuFont = Font(getMenuFont(), 14, 8);
	auto newDims = _menuFrame.DrawTextRegular(_menuFont, "New", 2, 1, 1, _lastButton == MenuButtons::New);
	auto openDims = _menuFrame.DrawTextRegular(_menuFont, "Open", 5 + newDims.b_x, 1, 1, _lastButton == MenuButtons::Open);
	auto saveDims = _menuFrame.DrawTextRegular(_menuFont, "Save", 5 + openDims.b_x, 1, 1, _lastButton == MenuButtons::Save);
	auto markerDims = _menuFrame.DrawRect(4 + saveDims.b_x, 4, 4 + saveDims.b_x + 8, 4 + 8, _useMarker ? 3 : 4);
	_menuFrame.DrawTextRegular(_menuFont, "By Goshante", 78 + markerDims.b_x, 1, 2);

	if (!firstDraw)
	{
		_menuButtons.push_back(newDims);
		_menuButtons.push_back(openDims);
		_menuButtons.push_back(saveDims);
		_menuButtons.push_back(markerDims);
		firstDraw = true;
	}
}

void Canvas::_draw()
{
	try
	{
		_pw = std::make_shared<pw::PixelWindow>(_width * _scale, (_height + MenuHeight) * _scale, _title.c_str());
		_pw->setOwner(this);
		_pw->addMouseCallback(&callbackMouse);
		_pw->setCloseCallback(&callbackClose);
		_pw->addCursorPosCallback(callbackCursor);
		_lastButton = MenuButtons::None;
		_menuFrame.ReInit(_width, _height);
		_holdDraw = false;
		_doDraw = 0;
		_redrawMenu();
		_reinit.reset();

		::SetForegroundWindow(GetHWND());
		::SetFocus(GetHWND());
		::SetActiveWindow(GetHWND());

		while (!_closed && _pw->isActive())
		{
			Canvas::TimePoint now = std::chrono::system_clock::now();
			auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
			auto last_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(_lastMenuClick);
			if (now_ms - last_ms > 350ms)
			{
				_lastButton = MenuButtons::None;
				_redrawMenu();
			}

			_pw->makeCurrent();

			_pw->pollEvents();
			_pw->beginFrame();

			_pw->setBackgroundColor(_background);

			_lock.lock();
			if (_frame[_pointY][_pointX] != 3 && _frame[_pointY][_pointX] != 5)
			{
				switch (_doDraw)
				{
				case 0:
					//DoNothing
					break;

				case 1:
					_frame[_pointY][_pointX] = 1;
					if (!_holdDraw)
						_doDraw = 0;
					break;

				case 2:
					_frame[_pointY][_pointX] = 0;
					if (!_holdDraw)
						_doDraw = 0;
					break;
				}
			}

			auto copy = _frame;
			auto menu = _menuFrame.GetBitmap();
			if (_useMarker)
				copy[_pointY][_pointX] = copy[_pointY][_pointX] == 0 ? 2 : 4;
			_lock.unlock();
			bmpUpscaleLinear(copy, _scale);
			bmpUpscaleLinear(menu, _scale);

			for (int y = 0; y < Min(MenuHeight * _scale, menu.size()); y++)
			{
				for (int x = 0; x < Min(_width * _scale, menu[y].size()); x++)
				{
					if (menu[y][x] == 1)
						_pw->setPixel(x, y, 0xFFFFFFFF);
					else if (menu[y][x] == 2)
						_pw->setPixel(x, y, 0xFFFF00FF);
					else if (menu[y][x] == 3)
						_pw->setPixel(x, y, 0xFF00FF00);
					else if (menu[y][x] == 4)
						_pw->setPixel(x, y, 0x55555555);
					else
						_pw->setPixel(x, y, 0xFF000044);
				}
			}

			for (int y = 0; y < Min(_height * _scale, copy.size()); y++)
			{
				for (int x = 0; x < Min(_width * _scale, copy[y].size()); x++)
				{
					if (copy[y][x] == 1)
						_pw->setPixel(x, y + MenuHeight * _scale, _brush);
					else if (copy[y][x] == 2)
						_pw->setPixel(x, y + MenuHeight * _scale, 0xFF00FF00);
					else if (copy[y][x] == 3)
						_pw->setPixel(x, y + MenuHeight * _scale, 0xFF0000FF);
					else if (copy[y][x] == 4)
						_pw->setPixel(x, y + MenuHeight * _scale, 0xFF008800);
					else if (copy[y][x] == 5)
						_pw->setPixel(x, y + MenuHeight * _scale, 0x55555555);
				}
			}

			_pw->endFrame();
		}

		_closed = true;
		_pw->forceClose();
		_pw.reset();	
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Exception", MB_OK | MB_ICONERROR);
		_closed = true;
		_pw.reset();
	}
}

void Canvas::SetCanvasCallback(MouseCallback callback)
{
	std::lock_guard<std::mutex> guard(_lock);
	_callbackCanvas = callback;
	_cc = true;
}

void Canvas::SetMenuCallback(MenuCallback callback)
{
	std::lock_guard<std::mutex> guard(_lock);
	_callbackMenu = callback;
	_mc = true;
}

void Canvas::SetCloseCallback(CloseCallback callback)
{
	std::lock_guard<std::mutex> guard(_lock);
	_callbackClose = callback;
	_clc = true;
}

bool Canvas::IsClosed() const
{
	if (_closed)
		_thread->join();
	return _closed && !_reinit;
}

HWND Canvas::GetHWND() const
{
	if (_closed)
		return NULL;

	return glfwGetWin32Window(_pw->_getHandle());
}

void Canvas::ReInit(const bitmap_t& bmp, int w, int h, int scale)
{
	std::lock_guard<std::mutex> guard(_lock);
	if (!_closed)
	{
		_reinit = std::make_shared<reinit_t>(reinit_t({ bmp, w, h, scale, false }));
		return;
	}
		
	_height = h;
	_width = w;
	_scale = scale;
	Show(bmp);
}

void Canvas::DoReinit()
{
	if (_reinit && !_reinit->invoked)
	{
		if (!_closed)
			Close();

		std::lock_guard<std::mutex> guard(_lock);
		_closed = false;
		_reinit->invoked = true;
		_height = _reinit->h;
		_width = _reinit->w;
		_scale = _reinit->sc;
		_frame = _reinit->pic;
		_pointY = 0;
		_pointX = 0;
		_thread = std::make_shared<std::thread>(&Canvas::_draw, this);
	}
}

bool Canvas::ReinitReady() const
{
	std::lock_guard<std::mutex> guard(_lock);
	return bool(_reinit);
}

void Canvas::SetOwner(void* ptr)
{
	_owner = ptr;
}