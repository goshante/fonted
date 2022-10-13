#include "Application.h"
#include "Font.h"
#include "version.h"

#include <sstream>

#define IDS_NEWWND						200
#define IDS_COLUMNS						201
#define IDS_UISCALE						202
#define IDS_CHARC						203
#define IDS_CHARH						204
#define IDS_CHARW						205
#define IDS_BTNCREATE					206

void MouseEvent(Canvas& canv, pw::mpos pos, int button, int action, int modes)
{
	bool hold = action == 1;
	if (button == 0) //Left click
	{
		if (canv.SetPoint((int)pos.x, (int)pos.y))
			canv.Draw(false, hold);
	}
	else if (button == 1) //Left click
	{
		if (canv.SetPoint((int)pos.x, (int)pos.y))
			canv.Draw(true, hold);
	}
}

void MenuEvent(Canvas& canv, Canvas::MenuButtons btn)
{
	switch (btn)
	{
	case Canvas::MenuButtons::New:
		canv.GetOwner<Application>()->_showNewWnd();
		break;

	case Canvas::MenuButtons::Open:
		EnableWindow(canv.GetHWND(), FALSE);
		try
		{
			auto fPath = browse(canv.GetHWND(), canv.GetHWND(), FileDialogType::Open);
			if (!fPath.empty())
				canv.GetOwner<Application>()->_loadFont(fPath);
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(NULL, ex.what(), "Font open failed", MB_OK | MB_ICONERROR);
		}
		EnableWindow(canv.GetHWND(), TRUE);
		break;

	case Canvas::MenuButtons::Save:
		EnableWindow(canv.GetHWND(), FALSE);
		try
		{
			auto fPath = browse(canv.GetHWND(), canv.GetHWND(), FileDialogType::Save);
			if (!fPath.empty())
				canv.GetOwner<Application>()->_saveFont(fPath);
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(NULL, ex.what(), "Font save failed", MB_OK | MB_ICONERROR);
		}
		EnableWindow(canv.GetHWND(), TRUE);
		break;
	}

	::SetForegroundWindow(canv.GetHWND());
	::SetFocus(canv.GetHWND());
	::SetActiveWindow(canv.GetHWND());
}

bool CloseEvent(Canvas& canv)
{
	return true;
}

Application::Application()
	: _columns(32)
	, _scale(5)
	, _chars(256)
	, _chW(5)
	, _chH(11)
{
	Font font = Font::makeEmptyFont(_chH, _chW, _chars);
	auto fbmp = font.getFontTable(_columns);
	_fontSeq.resize(_chars);
	for (utf8char_t i = 0; i < (utf8char_t)_chars; i++)
		_fontSeq[i] = i;
	_canvas = std::make_unique<Canvas>(int(fbmp[0].size()), int(fbmp.size()), _scale, (std::string("ASCII font editor ") + VERSION + " by Goshante").c_str(), false, 0xFFFFFF00);
	_canvas->SetOwner(this);
	_canvas->SetPicture(fbmp);
	_canvas->SetCanvasCallback(&MouseEvent);
	_canvas->SetMenuCallback(&MenuEvent);
	_canvas->SetCloseCallback(&CloseEvent);
}
Application::~Application()
{

}

bool Application::ProcessEventLoop()
{
	if (!_canvas->IsClosed())
	{
		if (_canvas->ReinitReady())
			_canvas->DoReinit();
		if (GetAsyncKeyState(VK_UP) & 0x01)
			_canvas->MovePoint(0, 1);

		if (GetAsyncKeyState(VK_DOWN) & 0x01)
			_canvas->MovePoint(0, -1);

		if (GetAsyncKeyState(VK_LEFT) & 0x01)
			_canvas->MovePoint(1, 0);

		if (GetAsyncKeyState(VK_RIGHT) & 0x01)
			_canvas->MovePoint(-1, 0);

		if (GetAsyncKeyState(VK_SPACE) & 0x01)
			_canvas->Draw(false);

		if (GetAsyncKeyState(VK_LCONTROL) & 0x01)
			_canvas->Draw(true);

		if (GetAsyncKeyState(0x43) & 0x01) //C
			_canvas->CopyCell(_chH, _chW, _chars);

		if (GetAsyncKeyState(0x56) & 0x01) //V
			_canvas->PasteCell(_chH, _chW, _chars);

		return true;
	}

	return false;
}

LRESULT CALLBACK NewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND btn, cols, scale, chars, charw, charh;
	Application* app = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	cols = GetDlgItem(hWnd, IDS_COLUMNS);
	scale = GetDlgItem(hWnd, IDS_UISCALE);
	chars = GetDlgItem(hWnd, IDS_CHARC);
	charw = GetDlgItem(hWnd, IDS_CHARW);
	charh = GetDlgItem(hWnd, IDS_CHARH);
	btn = GetDlgItem(hWnd, IDS_BTNCREATE);
	auto wp = LOWORD(wParam);

	int iCol, iScale, iCharC, iCharW, iCharH;
	char buf[32];
	GetWindowTextA(cols, buf, sizeof(buf));
	iCol = std::atoi(buf);
	GetWindowTextA(scale, buf, sizeof(buf));
	iScale = std::atoi(buf);
	GetWindowTextA(chars, buf, sizeof(buf));
	iCharC = std::atoi(buf);
	GetWindowTextA(charw, buf, sizeof(buf));
	iCharW = std::atoi(buf);
	GetWindowTextA(charh, buf, sizeof(buf));
	iCharH = std::atoi(buf);

	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		CloseWindow(hWnd);
		PostMessage(hWnd, WM_QUIT, 0, 0);
		break;

	case WM_COMMAND:
		switch (wp)
		{
		case IDS_BTNCREATE:
			if (app->_createWorkspace(hWnd, iCol, iCharH, iCharW, iCharC, iScale))
			{
				CloseWindow(hWnd);
				PostMessage(hWnd, WM_QUIT, 0, 0);
			}
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Application::_showNewWnd()
{
	const wchar_t CLASS_NAME[] = L"NewWn";
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSW wc = {};
	wc.lpfnWndProc = &NewWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDS_NEWWND));

	int width = 220;
	int height = 140;
	int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	HWND parent = _canvas->GetHWND();

	RegisterClassW(&wc);
	EnableWindow(parent, FALSE);
	HWND hwnd = CreateWindowExW(
		NULL,
		CLASS_NAME,
		L"New workspace",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN,
		xPos, yPos, width, height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd)
	{
		EnableWindow(parent, TRUE);
		throw (std::runtime_error("Failed to create window."));
	}

	//Begin constructing window
	int ox = 10, oy = 4;
	CreateWindowElement(hwnd, ET_STATIC, "Columns:", hInstance, WS_VISIBLE, NULL, NULL, ox, oy, 125, 20, false);
	CreateWindowElement(hwnd, ET_STATIC, "UI Scale:", hInstance, WS_VISIBLE, NULL, NULL, ox + 60, oy, 125, 20, false);
	CreateWindowElement(hwnd, ET_STATIC, "Characters:", hInstance, WS_VISIBLE, NULL, NULL, ox + 60 * 2, oy, 125, 20, false);
	oy += 17;
	CreateWindowElement(hwnd, ET_EDIT, "24", hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP, ES_NUMBER, HMENU(IDS_COLUMNS), ox, oy, 30, 23, false);
	CreateWindowElement(hwnd, ET_EDIT, "3", hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP, ES_NUMBER, HMENU(IDS_UISCALE), ox + 60, oy, 30, 23, false);
	CreateWindowElement(hwnd, ET_EDIT, "256", hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP, ES_NUMBER, HMENU(IDS_CHARC), ox + 60 * 2, oy, 30, 23, false);
	oy += 25;
	CreateWindowElement(hwnd, ET_STATIC, "Char W:", hInstance, WS_VISIBLE, NULL, NULL, ox, oy, 125, 20, false);
	CreateWindowElement(hwnd, ET_STATIC, "Char H:", hInstance, WS_VISIBLE, NULL, NULL, ox + 60, oy, 125, 20, false);
	oy += 17;
	CreateWindowElement(hwnd, ET_EDIT, "8", hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP, ES_NUMBER, HMENU(IDS_CHARW), ox, oy, 30, 23, false);
	CreateWindowElement(hwnd, ET_EDIT, "14", hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP, ES_NUMBER, HMENU(IDS_CHARH), ox + 60, oy, 30, 23, false);
	CreateWindowElement(hwnd, ET_BUTTON, "Create", hInstance, WS_VISIBLE | WS_TABSTOP | BS_FLAT, NULL, HMENU(IDS_BTNCREATE), ox + 60 * 2, oy - 1, 60, 25, false);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	auto a = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	EnableWindow(parent, TRUE);
	DestroyWindow(hwnd);
}

bool Application::_createWorkspace(HWND hwnd, int col, int h, int w, int count, int scale)
{
	auto errBox = [&hwnd](const std::string& msg)
	{
		MessageBoxA(hwnd, msg.c_str(), "Wrong value", MB_OK | MB_ICONEXCLAMATION);
	};

	if (!_canvas)
	{
		errBox("canv is nullptr");
		return false;
	}

	if (col < 24 || col > 64)
	{
		errBox("Column count cannot be lower than 24 or higher than 64.");
		return false;
	}

	if (h < 4 || h > 32)
	{
		errBox("Height cannot be lower than 4 or higher than 32.");
		return false;
	}
	
	if (w < 4 || w > 32)
	{
		errBox("Width cannot be lower than 4 or higher than 32.");
		return false;
	}

	if (scale < 1 || scale > 8)
	{
		errBox("Scale cannot be lower than 1 or higher than 8.");
		return false;
	}

	if (count < 1 || count > 256)
	{
		errBox("Char count cannot be lower than 1 or higher than 256.");
		return false;
	}

	_scale = scale;
	_chars = count;
	_columns = col;
	_chH = h;
	_chW = w;
	_fontSeq.resize(_chars);
	for (utf8char_t i = 0; i < (utf8char_t)_chars; i++)
		_fontSeq[i] = i;
	Font emptyFont = Font::makeEmptyFont(h, w, count);
	auto pic = emptyFont.getFontTable(col);
	_canvas->ReInit(pic, (int)pic[0].size(), (int)pic.size(), scale);

	return true;
}

void Application::_saveFont(const std::string& path)
{
	std::stringstream ss;
	ss << _chW << "x" << _chH << "\n";
	std::vector<std::string> intervals;

	auto ch2hex = [](utf8char_t ch) -> std::string
	{
		char buf[32];
		sprintf_s(buf, sizeof(buf), "0x%X", ch);
		return buf;
	};

	utf8char_t prev;
	size_t start = 0;
	for (size_t i = 0; i < _fontSeq.size(); i++)
	{
		auto ch = _fontSeq[i];
		if (i == 0)
		{
			prev = ch;
			continue;
		}

		if (ch - prev != 1)
		{
			size_t end = i - 1;
			if (end - start > 0)
			{
				std::stringstream sstr;
				sstr << _fontSeq[start] << "-" << _fontSeq[end];
				intervals.push_back(sstr.str());
			}
			else
				intervals.push_back(ch2hex(_fontSeq[start]));
			start = i;

			if (i == _fontSeq.size() - 1)
				intervals.push_back(ch2hex(ch));
		}
		else if (i == _fontSeq.size() - 1)
		{
			size_t end = i;
			std::stringstream sstr;
			sstr << _fontSeq[start] << "-" << _fontSeq[end];
			intervals.push_back(sstr.str());
		}
		prev = ch;
	}

	ss << "[";
	for (size_t i = 0; i < intervals.size(); i++)
	{
		if (i > 0)
			ss << ", ";
		ss << intervals[i];
	}
	ss << "]\n";
	auto font = _canvas->GetPicture();

	int cc = 0;
	for (size_t y = 0; y < font.size(); y += _chH + 1)
	{
		for (size_t x = 0; x < font[0].size(); x += _chW + 1)
		{
			if (cc >= _chars)
				break;

			for (size_t yy = 0; yy < _chH; yy++)
			{
				for (size_t xx = 0; xx < _chW; xx++)
				{
					pixel_t px = font[y + yy][x + xx];
					if (px == 0)
						ss << '0';
					else
						ss << '1';
				}
			}
			cc++;
		}
	}

	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Failed to open font file");

	DWORD dwWritten = 0;
	auto str = ss.str();
	if (!WriteFile(hFile, &str[0], (DWORD)str.length(), &dwWritten, NULL))
	{
		CloseHandle(hFile);
		throw std::runtime_error("Failed to write file");
	}
	CloseHandle(hFile);
}

void Application::_loadFont(const std::string& path)
{
	Font font(path);
	bitmap_t table = font.getFontTable(_columns);
	_fontSeq = font.GetAllSupportedChars();
	_chars = (int)_fontSeq.size();
	_chW = font.GetWidth();
	_chH = font.GetHeight();
	_canvas->ReInit(table, (int)table[0].size(), (int)table.size(), _scale);
}