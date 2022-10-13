#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include "Canvas.h"
#include "Utils.h"

class Application
{
private:
	std::unique_ptr<Canvas> _canvas;
	int _columns;
	int _scale;
	int _chars;
	int _chW;
	int _chH;
	std::vector<utf8char_t> _fontSeq;

	Application(Application&) = delete;
	Application& operator=(Application&) = delete;

	friend bool CloseEvent(Canvas& canv);
	friend void MenuEvent(Canvas& canv, Canvas::MenuButtons btn);
	friend void MouseEvent(Canvas& canv, pw::mpos pos, int button, int action, int modes);
	friend LRESULT CALLBACK NewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void _showNewWnd();
	bool _createWorkspace(HWND hwnd, int col, int h, int w, int count, int scale);
	void _saveFont(const std::string& path);
	void _loadFont(const std::string& path);

public:
	Application();
	~Application();

	bool ProcessEventLoop();
};