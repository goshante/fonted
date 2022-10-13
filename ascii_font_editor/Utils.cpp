#include "Utils.h"
#include <stdexcept>
#include <commctrl.h>
#include <richedit.h>
#include <tlhelp32.h>
#include <Shlwapi.h>
#include <Shlobj.h>

#ifdef _WIN64
using flexInt = long long;
using flexUint = unsigned long long;
#else
using flexInt = long;
using flexUint = unsigned long;
#endif

void InitBitmap(bitmap_t& bmp, int h, int w)
{
	bmp.clear();
	bmp.resize(h);
	for (auto& y : bmp)
	{
		y.resize(w);
		for (auto& x : y)
			x = 0;
	}
}

void RemoveBOMFromString(std::string& str)
{
	if (str.length() < 3)
		return;

	if (str[0] == 0xEF && str[1] == 0xBB && str[2] == 0xBF)
		str = str.substr(3, str.length() - 1);
}

size_t strlen_utf8(const std::string& u8str)
{
	size_t len = 0;
	for (size_t i = 0; i < u8str.length();)
	{
		int cplen = 1;
		if ((u8str[i] & 0xf8) == 0xf0) cplen = 4;
		else if ((u8str[i] & 0xf0) == 0xe0) cplen = 3;
		else if ((u8str[i] & 0xe0) == 0xc0) cplen = 2;
		if ((i + cplen) > u8str.length()) cplen = 1;

		len++;
		i += cplen;
	}

	return len;
}

void enumerateUTF8String(const std::string& u8str, std::function<void(utf8char_t ch, size_t n, size_t cpsz)> callback)
{
	size_t n = 0;
	for (size_t i = 0; i < u8str.length();)
	{
		int cplen = 1;
		if ((u8str[i] & 0xf8) == 0xf0) cplen = 4;
		else if ((u8str[i] & 0xf0) == 0xe0) cplen = 3;
		else if ((u8str[i] & 0xe0) == 0xc0) cplen = 2;
		if ((i + cplen) > u8str.length()) cplen = 1;
		utf8char_t ch = 0;
		auto sch = u8str.substr(i, cplen);
		for (size_t k = 0; k < cplen; k++)
			reinterpret_cast<char*>(&ch)[k] = sch[k];
		callback(ch, n, cplen);
		n++;
		i += cplen;
	}
}

std::string utf8char_to_stdString(utf8char_t ch)
{
	std::string str;
	size_t len = 0;
	char* u8str = (char*)&ch;
	int cplen = 1;
	if ((u8str[0] & 0xf8) == 0xf0) cplen = 4;
	else if ((u8str[0] & 0xf0) == 0xe0) cplen = 3;
	else if ((u8str[0] & 0xe0) == 0xc0) cplen = 2;

	for (size_t i = 0; i < cplen; i++)
		str.push_back(u8str[i]);

	return str;
}

std::vector<unsigned char> bmp2raw(const bitmap_t& bmp)
{
	if (bmp.size() == 0)
		return {};

	std::vector<unsigned char> raw(bmp.size() * bmp[0].size());
	for (size_t y = 0; y < bmp.size(); y++)
	{
		for (size_t x = 0; x < bmp[0].size(); x++)
		{
			raw[y + x] = bmp[y][x];
		}
	}

	return raw;
}

void bmpUpscaleLinear(bitmap_t& bmp, int scale)
{
	if (bmp.size() == 0 || scale <= 1)
		return;

	bitmap_t ups;
	InitBitmap(ups, int(bmp.size()) * scale, int(bmp[0].size()) * scale);

	for (int y = 0; y < bmp.size(); y++)
	{
		for (int x = 0; x < bmp[0].size(); x++)
		{
			for (int sy = 0; sy < scale; sy++)
			{
				for (int sx = 0; sx < scale; sx++)
				{
					ups[y * scale + sy][x * scale + sx] = bmp[y][x];
				}
			}
		}
	}

	bmp = ups;
}

HWND CreateWindowElement(HWND Parent, UINT Type, const char* Title, HINSTANCE hInst, DWORD Style, DWORD StyleEx, HMENU ElementID, INT pos_x, INT pos_y, INT Width, INT Height, BOOL NewRadioGroup)
{
	HWND hWnd = NULL;
	const char* ClassName = nullptr;

	Style |= WS_CHILD;

	switch (Type)
	{
	case ET_STATIC:
		ClassName = "STATIC";
		break;

	case ET_BUTTON:
		ClassName = "BUTTON";
		break;

	case ET_EDIT:
		ClassName = "EDIT";
		break;

	case ET_COMBOBOX:
		ClassName = "COMBOBOX";
		break;

	case ET_LISTBOX:
		ClassName = "LISTBOX";
		break;

	case ET_MDICLIENT:
		ClassName = "MDICLIENT";
		break;

	case ET_SCROLLBAR:
		ClassName = "SCROLLBAR";
		break;

	case ET_CHECKBOX:
		ClassName = "BUTTON";
		Style |= BS_AUTOCHECKBOX;
		break;

	case ET_RADIOBUTTON:
		ClassName = "BUTTON";
		Style |= BS_AUTORADIOBUTTON;
		if (NewRadioGroup)
			Style |= WS_GROUP;
		break;

	case ET_GROUPBOX:
		ClassName = "BUTTON";
		Style |= BS_GROUPBOX;
		break;

	case ET_PROGRESS:
		ClassName = PROGRESS_CLASSA;
		break;


	default: ClassName = "STATIC";
	}

	hWnd = CreateWindowExA(StyleEx, ClassName, Title, Style, pos_x, pos_y, Width, Height, Parent, ElementID, hInst, NULL);

	if (!hWnd)
		return nullptr;


	if (NewRadioGroup)
		CheckDlgButton(Parent, int((flexInt)ElementID), BST_CHECKED);

	HFONT hFont = CreateFontA(13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
	SendMessageA(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

	return hWnd;
}

std::string browse(HWND hwnd, HWND outputWindow, FileDialogType fdType)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);

	COMDLG_FILTERSPEC rgSpecOpen[] =
	{
		{ L"Font (*.fnt)" , L"*.fnt" },
		{ L"Text files (*.txt)" , L"*.txt" },
		{ L"All files (*.*)" , L"*.*" },
	};

	COMDLG_FILTERSPEC rgSpecSave[] =
	{
		{ L"Font (*.fnt)" , L"*.fnt" },
		{ L"Text files (*.txt)" , L"*.txt" }
	};

	const wchar_t defaultOpenFormat[] = L"*.txt;*.fnt";
	const wchar_t defaultSaveFormat[] = L"";
	FILEOPENDIALOGOPTIONS fopt = 0;
	bool succeed = false;

	if (SUCCEEDED(hr))
	{
		IFileDialog* pFileDialog;
		std::string astr;
		std::wstring wstr;

		if (fdType == FileDialogType::Save)
		{
			hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
				IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileDialog));
		}
		else
		{
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileDialog));
		}

		if (SUCCEEDED(hr))
		{
			pFileDialog->GetOptions(&fopt);

			if (fdType == FileDialogType::Save)
			{
				pFileDialog->SetFileTypes(ARRAYSIZE(rgSpecSave), rgSpecSave);
				pFileDialog->SetDefaultExtension(defaultSaveFormat);
				fopt |= FOS_PATHMUSTEXIST;
			}
			else if (fdType == FileDialogType::Open)
			{
				pFileDialog->SetFileTypes(ARRAYSIZE(rgSpecOpen), rgSpecOpen);
				pFileDialog->SetDefaultExtension(defaultOpenFormat);
				fopt |= FOS_FILEMUSTEXIST;
			}
			else
				fopt |= FOS_PICKFOLDERS;
			pFileDialog->SetFileTypeIndex(0);
			pFileDialog->SetOptions(fopt);

			hr = pFileDialog->Show(hwnd);

			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				wchar_t* pszFilePath = nullptr;
				hr = pFileDialog->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					if (SUCCEEDED(hr))
					{
						wstr = pszFilePath;
						CoTaskMemFree(pszFilePath);
						succeed = true;
					}

					pItem->Release();
				}
			}
			pFileDialog->Release();
		}
		CoUninitialize();

		if (succeed)
		{
			astr.resize(wstr.size() + 1);
			SetWindowTextW(outputWindow, wstr.c_str());
			GetWindowTextA(outputWindow, &astr[0], int(astr.size()));
		}

		return astr;
	}

	return "";
}