#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include "Canvas.h"
#include "Utils.h"

class FontTestWindow;

class Application
{
private:
	std::unique_ptr<Canvas> _canvas;
	int _columns;
	int _scale;
	int _chars;
	int _chW;
	int _chH;
	int _fontInterval;
	std::vector<utf8char_t> _fontSeq;
	std::atomic<bool> _testingFont;
	std::unique_ptr<std::thread> _testThread;

	Application(Application&) = delete;
	Application& operator=(Application&) = delete;

	friend bool CloseEvent(Canvas& canv);
	friend void MenuEvent(Canvas& canv, Canvas::MenuButtons btn);
	friend void MouseEvent(Canvas& canv, pw::mpos pos, int button, int action, int modes);
	friend LRESULT CALLBACK NewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void _testFont();
	void _showNewWnd();
	bool _createWorkspace(HWND hwnd, const std::string& sequence, int col, int h, int w, int interval, int count, int scale);
	void _saveFont(const std::string& path);
	void _loadFont(const std::string& path);
	std::string _makeFontDict();

public:
	Application();
	~Application();

	bool ProcessEventLoop();
};