#pragma once
#include <vector>
#include <string>
#include <PixelWindow/PixelWindow.h>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>

#include "VirtualCanvas.h"
#include "Font.h"
#include "Utils.h"

class Canvas
{
public:
	enum class MenuButtons
	{
		New,
		Open,
		Save,
		MarkerSwitch,
		None
	};

	using MouseCallback = std::function<void(Canvas& canv, pw::mpos, int, int, int)>;
	using MenuCallback = std::function<void(Canvas& canv, MenuButtons)>;
	using CloseCallback = std::function<bool(Canvas& canv)>;
	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

protected:
	struct reinit_t
	{
		bitmap_t pic;
		int w;
		int h;
		int sc;
		bool invoked;
	};

	std::shared_ptr<pw::PixelWindow>	_pw;
	std::shared_ptr<std::thread>		_thread;
	std::string							_title;
	mutable std::mutex					_lock;
	bitmap_t							_frame;
	int									_width;
	int									_height;
	uint32_t							_background;
	uint32_t							_brush;
	std::atomic<bool>					_closed;
	int									_pointX;
	int									_pointY;
	int									_scale;
	std::atomic<uint8_t>				_doDraw;
	std::atomic<bool>					_cc;
	std::atomic<bool>					_mc;
	std::atomic<bool>					_clc;
	std::atomic<bool>					_useMarker;
	MouseCallback						_callbackCanvas;
	MenuCallback						_callbackMenu;
	CloseCallback						_callbackClose;
	Font								_menuFont;
	VirtualCanvas						_menuFrame;
	std::vector<VirtualCanvas::Dims>	_menuButtons;
	MenuButtons							_lastButton;
	TimePoint							_lastMenuClick;
	std::shared_ptr<reinit_t>			_reinit;
	void*								_owner;
	std::atomic<bool>					_holdDraw;
	TimePoint							_lastDrawCall;

	Canvas(Canvas&) = delete;
	Canvas& operator=(Canvas&) = delete;

	void _draw();
	void _redrawMenu();

	friend void callbackMouse(void* owner, pw::mpos pos, int button, int action, int modes);
	friend bool callbackClose(void* owner);
	friend void callbackCursor(void* owner, pw::mpos pos);

public:
	Canvas(int w, int h, int scale = 1, const std::string& title = "Preview", bool startHidden = false, uint32_t brush = 0xFFFFFF00, uint32_t background = 0x00000000);
	~Canvas();

	bitmap_t GetPicture() const;
	bool IsClosed() const;
	HWND GetHWND() const;
	bool ReinitReady() const;
	template<class T>
	T* GetOwner() const { return reinterpret_cast<T*>(_owner); }

	void SetPicture(const bitmap_t& picture);
	void Show(const bitmap_t& bmp = {});
	void Close();
	void MovePoint(int x, int y);
	bool SetPoint(int x, int y);
	void Draw(bool erase, bool hold = false);
	void SetCanvasCallback(MouseCallback callback);
	void SetMenuCallback(MenuCallback callback);
	void SetCloseCallback(CloseCallback callback);
	void ReInit(const bitmap_t& bmp, int w, int h, int scale);
	void DoReinit();
	void SetOwner(void* ptr);
};